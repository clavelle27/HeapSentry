#include <linux/module.h>
#include <linux/unistd.h>

MODULE_LICENSE("GPL");

// The value of this variable differs in each machine. For security
// reasons, the value of the sys_call_table is not exported in contemporary
// kernels. This address can be dynamically found out (with some effort).
// For the purposes of this project, however, it is not required.
//
// This value is found by running the following command at the prompt.
// $sudo cat /boot/System.map-3.13.0-61-generic | grep sys_call_table
// The suffix after "System.map-" should be the result of "$uname -r".
//
// x86_64: sys_call_table
// #define SYS_CALL_TABLE_ADDRESS 0xffffffff81801400

// Address of ia32_sys_call_table. with x86_64 table address, `int $0x80` did not work.
// Ideally, `syscall` should be used with a recompiled kernel having a new system
// call added. However, since the goal is just to introduce a communication channel
// between heapsentryu and heapsentryk, 32 bit sys_call_table is used in this module.
// ia-32: ia32_sys_call_table
// #define SYS_CALL_TABLE_ADDRESS 0xffffffff81809ca0
//
// Note: This value now comes from makefile to make it configurable.

// Look into /usr/include/asm-generic/unistd.h for all the system call numbers.
// The array holding function pointers to system calls in kernel is indexed by these
// numbers. These are placeholders (I guess), for new calls and the array is not completely
// filled up. One such `hole` is in the range 279-1023. So, using 280 here. This is the
// system call number which userspace needs to invoke to reach heapsentryk's system call.
// #define SYS_CALL_NUMBER 280
//
// This comes from makefile now to make it configurable.

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
	size_t *group_buffer = (size_t *)v2;
	size_t group_count = (size_t)v3;
	int i = 0;
	printk("heapsentryk:received: group_buffer:[%p] group_count:[%ld]\n",
	       group_buffer, group_count);
	for (i = 0; i < group_count; i++) {
		printk("buf[%d][0]:[%p] buf[%d][1]:[%ld] deref:[%d]\n", i, (void *)*(group_buffer + i * 2),i,
		       *(group_buffer + i * 2 + 1), *((int*)*(group_buffer + i * 2)));
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
