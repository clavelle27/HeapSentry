#include <linux/module.h>
#include <linux/unistd.h>

MODULE_LICENSE("GPL");

// The value of this variable differs in each machine. For security
// reasons, the value of the sys_call_table is not exported in contemporary
// kernels. This address can be dynamically found out (with some effort).
// For the purposes of this project, however, it is not required.
//
// This value can be found by running the following command at the prompt.
// $sudo cat /boot/System.map-3.13.0-61-generic | grep sys_call_table
// The suffix after "System.map-" should be the result of "$uname -r".
#define SYS_CALL_TABLE_ADDRESS 0xffffffff81801400

void **sys_call_table;

void set_read_write(long unsigned int _addr);

asmlinkage void* (*original_mmap) (void *addr, size_t length, int prot, int flags,
		      int fd, off_t offset);
asmlinkage int (*original_munmap) (void *addr, size_t length);

// Init function of this kernel module. This gets executed when the module is
// loaded into the kernel.
static int __init mod_entry_func(void)
{
	printk(KERN_INFO "heapsentryk entering...\n");

	// Setting the address of the system call table here.
	sys_call_table = (void *)SYS_CALL_TABLE_ADDRESS;

	// Giving write permissions to the page containing this table.
	set_read_write(SYS_CALL_TABLE_ADDRESS);

	// Caching the original addresses of mmap() and munmap() system calls.
	// These place holders in the table will be changed to hold the addresses
	// of heapsentryk's mmap() and munmap().
	//
	// Table should be restored to contain its original addresses when this
	// module in unloaded.
	original_mmap = sys_call_table[__NR_mmap];
	original_munmap = sys_call_table[__NR_munmap];

	printk(KERN_INFO "__NR_mmap:%d\n", __NR_mmap);
	printk(KERN_INFO "__NR_munmap:%d\n", __NR_munmap);
	printk(KERN_INFO "original_mmap:%p\n",original_mmap);
	printk(KERN_INFO "original_munmap:%p\n",original_munmap);
	return 0;
}

// Exit function of this kernel module. This gets executed when this module
// is unloaded from kernel.
static void __exit mod_exit_func(void)
{
	// Restoring the original addresses of the system calls in the table.
	sys_call_table[__NR_mmap] = original_mmap;
	sys_call_table[__NR_munmap] = original_munmap;

	printk(KERN_INFO "heapsentryk exiting...\n");
}

// Gives read-write permissions for the page pointed by the given address.
void set_read_write(long unsigned int _addr)
{
	unsigned int level;
	pte_t *pte = lookup_address(_addr, &level);
	if (pte->pte & ~_PAGE_RW)
		pte->pte |= _PAGE_RW;
}

module_init(mod_entry_func);
module_exit(mod_exit_func);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("heapsentry@cs.stonybrook.edu");
MODULE_DESCRIPTION("HeapSentry Kernel Module");
