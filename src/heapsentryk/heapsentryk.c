#include <linux/module.h>
#include <linux/unistd.h>

MODULE_LICENSE("GPL");

void **sys_call_table;

void set_read_write(long unsigned int _addr);

asmlinkage void *(*original_mmap) (void *addr, size_t length, int prot,
				   int flags, int fd, off_t offset);
asmlinkage void *heapsentryk_mmap(void *addr, size_t length, int prot,
				  int flags, int fd, off_t offset);
asmlinkage int (*original_munmap) (void *addr, size_t length);
asmlinkage void (*original_exit) (int status);
asmlinkage int heapsentryk_munmap(void *addr, size_t length);
asmlinkage size_t sys_heapsentryk_canary(size_t not_used, size_t v2, size_t v3);

// System call which receives the canary information from heapsentryu
// and stores it in its symbol table. This information is used by
// high-risk calls to verify the canaries.
asmlinkage size_t sys_heapsentryk_canary(size_t not_used, size_t v2, size_t v3)
{
	size_t *p_group_buffer = (size_t *)v2;
	size_t *p_group_count = (size_t *)v3;
	size_t i = 0;
	printk("heapsentryk:received: p_group_buffer:[%p] p_group_count:[%p] \n",
	       p_group_buffer, p_group_count);
	printk("heapsentryk: dereferencing p_group_count:[%ld]\n",*p_group_count);
	for (i = 0; i < *p_group_count; i++) {
		printk("buf[%ld][0]:[%p] buf[%ld][1]:[%ld] deref:[%ld]\n", i, (void *)*(p_group_buffer + i * 2),i,
		       *(p_group_buffer + i * 2 + 1), *((size_t*)*(p_group_buffer + i * 2)));
	}
	// TODO: Change the parameters to receive the canary information in bulk.
	// This bulk count will be user configurable in the userspace.
	// 
	// Implement Hash tables or find out a good library which does our job easier.
	return 30;
}

asmlinkage int heapsentryk_munmap(void *addr, size_t length)
{
	//printk(KERN_INFO "Entered heapsentryk_munmap()\n");
	return original_munmap(addr, length);
}

asmlinkage void *heapsentryk_mmap(void *addr, size_t length,
				  int prot, int flags, int fd, off_t offset)
{
	//printk("KERN_INFO Entered heapsentryk_mmap()\n");
	return original_mmap(addr, length, prot, flags, fd, offset);
}

// Init function of this kernel module. This gets executed when the module is
// loaded into the kernel.
static int __init mod_entry_func(void)
{
	//printk(KERN_INFO "heapsentryk entering...\n");

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
	// Substituting the system calls with heapsentryk's calls.
	// In these substituted function, decision can be taken regarding further
	// actions. Either exiting the process or letting the request through to
	// original syscalls.
	sys_call_table[__NR_mmap] = heapsentryk_mmap;
	sys_call_table[__NR_munmap] = heapsentryk_munmap;
	// Setting HeapSentry system call to sys_call_table to the configured index.
	sys_call_table[SYS_CALL_NUMBER] = sys_heapsentryk_canary;
	return 0;
}

// Exit function of this kernel module. This gets executed when this module
// is unloaded from kernel.
static void __exit mod_exit_func(void)
{
	// Restoring the original addresses of the system calls in the table.
	sys_call_table[__NR_mmap] = original_mmap;
	sys_call_table[__NR_munmap] = original_munmap;
	sys_call_table[SYS_CALL_NUMBER] = 0;
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
