#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/prctl.h>

MODULE_LICENSE("GPL");

// Look for system call numbers in /usr/include/asm-generic/unistd.h
// Look for hashtable options in /usr/src/linux-headers-3.19.0-25/include/linux/hashtable.h

// Hashtable to store canary information
DEFINE_HASHTABLE(buckets, 20);

void **sys_call_table;

void set_read_write(long unsigned int _addr);

struct canary_entry {
	size_t canary_location;
	size_t canary_value;
	struct hlist_node next;
};

struct pid_entry {
	struct hlist_head* p_process_hashtable;
	struct list_head pid_list_head;
}; 

asmlinkage int (*original_fork) (void);
asmlinkage int (*original_chmod) (const char *pathname, int mode);
asmlinkage int (*original_munmap) (void *addr, size_t length);
asmlinkage int (*original_execve)(const char*, char *const argv[], char *const envp[]);
asmlinkage int (*original_open) (const char *, int, int);
asmlinkage void *(*original_mmap) (void *addr, size_t length, int prot,
				   int flags, int fd, off_t offset);
asmlinkage void (*original_exit) (int status);
asmlinkage int (*original_getpid) (void);

asmlinkage void *heapsentryk_mmap(void *addr, size_t length, int prot,
				  int flags, int fd, off_t offset);
asmlinkage int heapsentryk_munmap(void *addr, size_t length);
asmlinkage int heapsentryk_execve(const char*, char *const argv[], char *const envp[]);
asmlinkage int heapsentryk_open (const char *, int, int);
asmlinkage int heapsentryk_fork (void);
asmlinkage int heapsentryk_chmod (const char *pathname, int mode);

asmlinkage size_t sys_heapsentryk_canary(size_t not_used, size_t v2, size_t v3);

// System call which receives the canary information from heapsentryu
// and stores it in its symbol table. This information is used by
// high-risk calls to verify the canaries.
asmlinkage size_t sys_heapsentryk_canary(size_t not_used, size_t v2, size_t v3)
{
	int bucket_index = 0;
	struct canary_entry *p_canary_entry = NULL;
	struct canary_entry *entry = NULL;
	size_t *p_group_buffer = (size_t *)v2;
	size_t *p_group_count = (size_t *)v3;
	size_t i = 0;


	struct pid_entry pid_list_entry;
	LIST_HEAD(process_list_head);
	struct list_head * position = NULL;
	struct pid_entry * p_pid_entry = NULL;

	printk("heapsentryk:received: p_group_buffer:[%p] p_group_count:[%p] \n",
	       p_group_buffer, p_group_count);
	printk("heapsentryk: dereferencing p_group_count:[%d]\n",*p_group_count);
	for (i = 0; i < *p_group_count; i++) {
		// Allocate an object to store in Hash table that persists beyond the scope of this function.
		entry = (struct canary_entry *) kmalloc(sizeof(struct canary_entry), GFP_KERNEL);
		memset((void *)entry, 0, sizeof(struct canary_entry));
		entry->canary_location = *(p_group_buffer + i * 2);
		entry->canary_value = *((size_t*)*(p_group_buffer + i * 2));
		hash_add(buckets, &entry->next, entry->canary_location);
		printk("buf[%d][0]:[%p] buf[%d][1]:[%d] deref:[%d]\n", i, (void *)*(p_group_buffer + i * 2),i,
		       *(p_group_buffer + i * 2 + 1), *((size_t*)*(p_group_buffer + i * 2)));
	}

	printk("Hashtable iteration started. buckets:%p\n",buckets);
	hash_for_each(buckets, bucket_index, p_canary_entry, next){
		printk(KERN_INFO "canary_location=%d canary_value=%d deref=%d is in bucket %d\n", p_canary_entry->canary_location, p_canary_entry->canary_value, *((size_t*)p_canary_entry->canary_location),bucket_index);
	}
	printk(KERN_INFO "sys_canary() from PID:%d\n",original_getpid());
	printk("Hashtable iteration ended\n");


	printk("Start: Linked list for PID<-->Hashtable information\n");
	pid_list_entry.p_process_hashtable = buckets;
	INIT_LIST_HEAD(&pid_list_entry.pid_list_head);
	list_add(&pid_list_entry.pid_list_head, &process_list_head);
	list_for_each(position, &process_list_head)
	{
		p_pid_entry = list_entry(position, struct pid_entry, pid_list_head);
		printk("p_process_hashtable: %p\n",p_pid_entry->p_process_hashtable);
	}
	printk("End: Linked list for PID<-->Hashtable information\n");

	// TODO: Change the parameters to receive the canary information in bulk.
	// This bulk count will be user configurable in the userspace.
	// 
	// Implement Hash tables or find out a good library which does our job easier.
	return 30;
}

asmlinkage int heapsentryk_execve(const char* filename, char *const argv[], char *const envp[])
{
	printk(KERN_INFO "Entered heapsentryk_execve()\n");
	return original_execve(filename, argv, envp);
}

asmlinkage int heapsentryk_open (const char *pathname, int flags, int mode)
{
	//printk(KERN_INFO "Entered heapsentryk_open()\n");
	return original_open(pathname, flags, mode);
}

asmlinkage int heapsentryk_fork (void)
{
	//printk(KERN_INFO "Entered heapsentryk_fork()\n");
	return original_fork();
}

asmlinkage int heapsentryk_chmod (const char *pathname, int mode)
{
	//printk(KERN_INFO "Entered heapsentryk_chmod()\n");
	return original_chmod(pathname, mode);
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
	printk(KERN_INFO "heapsentryk entered!\n");

	// Setting the address of the system call table here.
	sys_call_table = (void *)SYS_CALL_TABLE_ADDRESS;
	// Giving write permissions to the page containing this table.
	set_read_write(SYS_CALL_TABLE_ADDRESS);

	// Caching the original addresses of system calls.
	// These place holders in the table will be changed to hold the addresses
	// of heapsentryk's overriden system calls.
	//
	// Table should be restored to contain its original addresses when this
	// module in unloaded.
	original_mmap = sys_call_table[__NR_mmap];
	original_munmap = sys_call_table[__NR_munmap];
	original_execve = sys_call_table[__NR_execve];
	original_open = sys_call_table[__NR_open];
	original_fork = sys_call_table[__NR_fork];
	original_chmod = sys_call_table[__NR_chmod];
	original_getpid = sys_call_table[__NR_getpid];

	// Substituting the system calls with heapsentryk's calls.
	// In these substituted function, decision can be taken regarding further
	// actions. Either exiting the process or letting the request through to
	// original syscalls.
	sys_call_table[__NR_mmap] = heapsentryk_mmap;
	sys_call_table[__NR_munmap] = heapsentryk_munmap;
	sys_call_table[__NR_open] = heapsentryk_open;
	sys_call_table[__NR_execve] = heapsentryk_execve;
	sys_call_table[__NR_fork] = heapsentryk_fork;
	sys_call_table[__NR_chmod] = heapsentryk_chmod;

	// Setting HeapSentry system call to sys_call_table to the configured index.
	sys_call_table[SYS_CALL_NUMBER] = sys_heapsentryk_canary;

	// Initialize the Hashtable to store the canary information.
	hash_init(buckets);

	printk(KERN_INFO "Init PID: %d\n",original_getpid());

	return 0;
}

// Exit function of this kernel module. This gets executed when this module
// is unloaded from kernel.
static void __exit mod_exit_func(void)
{
	// Restoring the original addresses of the system calls in the table.
	sys_call_table[__NR_mmap] = original_mmap;
	sys_call_table[__NR_munmap] = original_munmap;
	sys_call_table[__NR_execve] = original_execve;
	sys_call_table[__NR_open] = original_open;
	sys_call_table[__NR_fork] = original_fork;
	sys_call_table[__NR_chmod] = original_chmod;
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
