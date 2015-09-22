#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

MODULE_LICENSE("GPL");

#define SYS_CALL_TABLE_ADDRESS 0xffffffff81801400
void **sys_call_table;

void set_page_rw(long unsigned int _addr);

static int __init mod_entry_func(void)
{
	printk(KERN_INFO "heapsentryk entering...\n");
	printk(KERN_INFO "sys_open: %p\n",sys_open);
	printk(KERN_INFO "__NR_open:%d\n", __NR_open);
	//printk(KERN_INFO "sys_call_table[__NR_open]:%p\n", sys_call_table[__NR_open]);
	return 0;
}

static void __exit mod_exit_func(void)
{
	sys_call_table = (void *)SYS_CALL_TABLE_ADDRESS;
	set_page_rw(SYS_CALL_TABLE_ADDRESS);
	printk(KERN_INFO "heapsentryk exiting...\n");
}

void set_page_rw(long unsigned int _addr)
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
