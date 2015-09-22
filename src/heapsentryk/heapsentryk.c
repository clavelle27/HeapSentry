#include <linux/module.h>

MODULE_LICENSE("GPL v2");

static int __init mod_entry_func(void)
{
	return 0;
}

static void __exit mod_exit_func(void)
{
}

module_init(mod_entry_func);
module_exit(mod_exit_func);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("heapsentry@cs.stonybrook.edu");
MODULE_DESCRIPTION("HeapSentry Kernel Module");
