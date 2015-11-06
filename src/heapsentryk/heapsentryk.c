#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/prctl.h>

MODULE_LICENSE("GPL");

// Look for system call numbers in /usr/include/asm-generic/unistd.h
// Look for hashtable options in /usr/src/linux-headers-3.19.0-25/include/linux/hashtable.h
// Look for list options in /usr/src/linux-headers-3.19.0-25/include/linux/list.h

void **sys_call_table;

// This sets the bucket size for a process specific hashtable.
// The size would be [1<<BUCKET_BITS_SIZE].
// Example: Value of 5 implies that the size is 32.
#define BUCKET_BITS_SIZE 5

// Define a linked list that holds information that connects PID with a process specific hashtable
LIST_HEAD(pid_list);

// The object that the link list holds.
typedef struct pid_entry {
	int pid;
	struct hlist_head *p_process_hashtable;
	struct list_head pid_list_head;
} Pid_entry;

// The object identified by a key(canary_location) in hashtable which holds canary information.
typedef struct canary_entry {
	size_t canary_location;
	size_t canary_value;
	struct hlist_node next;
} Canary_entry;

asmlinkage int (*original_fork) (void);
asmlinkage int (*original_chmod) (const char *pathname, int mode);
asmlinkage int (*original_munmap) (void *addr, size_t length);
asmlinkage int (*original_execve) (const char *, char *const argv[],
				   char *const envp[]);
asmlinkage int (*original_open) (const char *, int, int);
asmlinkage void *(*original_mmap) (void *addr, size_t length, int prot,
				   int flags, int fd, off_t offset);
asmlinkage void (*original_exit) (int status);
asmlinkage int (*original_getpid) (void);

asmlinkage void *heapsentryk_mmap(void *addr, size_t length, int prot,
				  int flags, int fd, off_t offset);
asmlinkage int heapsentryk_munmap(void *addr, size_t length);
asmlinkage int heapsentryk_execve(const char *, char *const argv[],
				  char *const envp[]);
asmlinkage int heapsentryk_open(const char *, int, int);
asmlinkage int heapsentryk_fork(void);
asmlinkage int heapsentryk_chmod(const char *pathname, int mode);

void set_read_write(long unsigned int _addr);

asmlinkage Canary_entry *find_canary_entry(struct
					   hlist_head (*hashtable)[1 <<
								   BUCKET_BITS_SIZE]);
asmlinkage Pid_entry *find_pid_entry(int pid);
asmlinkage size_t sys_heapsentryk_canary(size_t not_used, size_t v2, size_t v3);

asmlinkage Pid_entry *find_pid_entry(int pid)
{
	Pid_entry *entry = NULL;
	struct list_head *position = NULL;
	Pid_entry *p_pid_entry = NULL;

	list_for_each(position, &pid_list) {
		p_pid_entry = list_entry(position, Pid_entry, pid_list_head);
		if (pid == p_pid_entry->pid) {
			entry = p_pid_entry;
			break;
		}
	}
	return entry;
}

asmlinkage Canary_entry *find_canary_entry(struct
					   hlist_head (*hashtable)[1 <<
								  BUCKET_BITS_SIZE])
{
	int bucket_index = 0;
	Canary_entry *p_canary_entry = NULL;
	hash_for_each((*hashtable), bucket_index, p_canary_entry, next) {
		printk(KERN_INFO
		       "canary_location=%d canary_value=%d deref=%d is in bucket %d\n",
		       p_canary_entry->canary_location,
		       p_canary_entry->canary_value,
		       *((size_t *) p_canary_entry->canary_location),
		       bucket_index);
	}
	return NULL;
}

// System call which receives the canary information from heapsentryu
// and stores it in its symbol table. This information is used by
// high-risk calls to verify the canaries.
asmlinkage size_t sys_heapsentryk_canary(size_t not_used, size_t v2, size_t v3)
{
	size_t *p_group_buffer = (size_t *) v2;
	size_t *p_group_count = (size_t *) v3;
	struct hlist_head (*p_hash)[1 << BUCKET_BITS_SIZE] = NULL;

	Pid_entry *p_pid_entry = NULL;

	/*
	   printk("buf[%d][0]:[%p] buf[%d][1]:[%d] deref:[%d]\n", i,
	   (void *)*(p_group_buffer + i * 2), i,
	   *(p_group_buffer + i * 2 + 1),
	   *((size_t *) * (p_group_buffer + i * 2)));
	 */
	printk
	    ("heapsentryk:received: p_group_buffer:[%p] p_group_count:[%p] \n",
	     p_group_buffer, p_group_count);
	printk("heapsentryk: dereferencing p_group_count:[%d]\n",
	       *p_group_count);

	// Checking if there is an entry in the linked list corresponding to the current process.
	p_pid_entry = find_pid_entry(original_getpid());
	if (!p_pid_entry) {
		Pid_entry * p_pid_entry = (Pid_entry *) kmalloc(sizeof(Pid_entry), GFP_KERNEL);
		struct hlist_head (*buckets)[1 << BUCKET_BITS_SIZE] = (struct hlist_head (*)[1 << BUCKET_BITS_SIZE]) kmalloc(sizeof(struct hlist_head (*)[1 << BUCKET_BITS_SIZE]), GFP_KERNEL);
		size_t i = 0;
		
		printk("pid_entry not found for pid:%d\n", original_getpid());
		
		// Braces are added to void bugs that could arise by macro substitutions.
		// Since there is an asterisk in the argument, if the macro places the
		// argument as a suffix, there will be a problem.
		hash_init((*buckets));

		// Adding canaries to hashtable.
		for (i = 0; i < *p_group_count; i++) {
			Canary_entry *entry =
			    (Canary_entry *) kmalloc(sizeof(Canary_entry),
						     GFP_KERNEL);
			memset((void *)entry, 0, sizeof(Canary_entry));
			entry->canary_location = *(p_group_buffer + i * 2);
			entry->canary_value =
			    *((size_t *) * (p_group_buffer + i * 2));
			hash_add((*buckets), &entry->next, entry->canary_location);
		}
		p_hash = buckets;
		find_canary_entry(p_hash);

		// An object that connects PIDs to its corresponding hashtable.
		p_pid_entry->pid = original_getpid();
		p_pid_entry->p_process_hashtable = *buckets;
		INIT_LIST_HEAD(&p_pid_entry->pid_list_head);
		// Adding the object 'pid_list_entry' into linked list by name 'pid_list'
		list_add(&p_pid_entry->pid_list_head, &pid_list);
	} else {
		printk("Found pid_entry:%p found for pid:%d\n", p_pid_entry,
		       original_getpid());
		// Add the canary information into existing hashtable.
	}

	p_pid_entry = find_pid_entry(original_getpid());
	if (!p_pid_entry) {
		printk("pid_entry not found for pid:%d\n", original_getpid());
	}else{
		printk("Found pid_entry:%p found for pid:%d\n", p_pid_entry,
		       original_getpid());
	}

	return 0;
}

asmlinkage int heapsentryk_execve(const char *filename, char *const argv[],
				  char *const envp[])
{
	//printk(KERN_INFO "Entered heapsentryk_execve()\n");
	return original_execve(filename, argv, envp);
}

asmlinkage int heapsentryk_open(const char *pathname, int flags, int mode)
{
	//printk(KERN_INFO "Entered heapsentryk_open()\n");
	return original_open(pathname, flags, mode);
}

asmlinkage int heapsentryk_fork(void)
{
	//printk(KERN_INFO "Entered heapsentryk_fork()\n");
	return original_fork();
}

asmlinkage int heapsentryk_chmod(const char *pathname, int mode)
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
