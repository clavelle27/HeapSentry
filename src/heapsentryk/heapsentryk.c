#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/hashtable.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/prctl.h>

MODULE_LICENSE("GPL");

// Look for system call numbers in /usr/include/asm-generic/unistd.h
// Look for hashtable options in /usr/src/linux-headers-3.19.0-25/include/linux/hashtable.h
// Look for list options in /usr/src/linux-headers-3.19.0-25/include/linux/list.h
// Look for system calls syntax in /usr/src/linux-headers-3.19.0-25/include/linux/syscalls.h

void **sys_call_table;

// This sets the bucket size for a process specific hashtable.
// The size would be [1<<BUCKET_BITS_SIZE].
// Example: Value of 5 implies that the size is 32.
#define BUCKET_BITS_SIZE 5

// Define a linked list that holds information that connects PID with a process specific hashtable
LIST_HEAD(pid_list);

// The object that the link list holds.
typedef struct pid_entry {
	size_t *p_group_buffer;
	size_t *p_group_count;
	long pid;
	struct hlist_head (*p_process_hashtable)[1 << BUCKET_BITS_SIZE];
	struct list_head pid_list_head;
} Pid_entry;

// The object identified by a key(canary_location) in hashtable which holds canary information.
typedef struct canary_entry {
	size_t canary_location;
	size_t canary_value;
	struct hlist_node next;
} Canary_entry;

asmlinkage long (*original_fchmod) (unsigned int a1, umode_t a2);
asmlinkage long (*original_chmod) (const char __user * a1, umode_t a2);
asmlinkage long (*original_execve) (const char __user * a1,
				    const char __user * const __user * a2,
				    const char __user * const __user * a3);
asmlinkage long (*original_open) (const char __user * a1, int a2, umode_t a3);
asmlinkage long (*original_exit) (int a1);
asmlinkage long (*original_getpid) (void);
asmlinkage long (*original_exit) (int a1);
asmlinkage long (*original_clone) (unsigned long a1, unsigned long a2,
				   int __user * a3, int __user * a4, int a5);

asmlinkage long heapsentryk_fchmod (unsigned int a1, umode_t a2);
asmlinkage long heapsentryk_chmod(const char __user * a1, umode_t a2);
asmlinkage long heapsentryk_execve(const char __user * a1,
				   const char __user * const __user * a2,
				   const char __user * const __user * a3);
asmlinkage long heapsentryk_open(const char __user * a1, int a2, umode_t a3);
asmlinkage long heapsentryk_getpid(void);
asmlinkage long heapsentryk_clone(unsigned long a1, unsigned long a2,
				  int __user * a3, int __user * a4, int a5);
asmlinkage long heapsentryk_exit (int a1);

void set_read_write(long unsigned int _addr);
asmlinkage Canary_entry *find_hashtable_entry(size_t canary_location, struct
					      hlist_head (*hashtable)[1 <<
								      BUCKET_BITS_SIZE]);
asmlinkage Canary_entry *list_canaries(struct
				       hlist_head (*hashtable)[1 <<
							       BUCKET_BITS_SIZE]);
asmlinkage void free_canaries(void);
asmlinkage Pid_entry *find_pid_entry(int pid);
asmlinkage size_t sys_heapsentryk_canary(void);
asmlinkage size_t sys_heapsentryk_canary_init(size_t not_used, size_t v2,
					      size_t v3);
asmlinkage void iterate_pid_list(void);
asmlinkage int verify_canaries(void);

asmlinkage long heapsentryk_fchmod (unsigned int a1, umode_t a2)
{
	printk(KERN_INFO "Entered heapsentryk_fchmod pid:%ld\n",
	       original_getpid());
	if (verify_canaries()) {
		heapsentryk_exit(1);
	}
	return original_fchmod(a1,a2);
}

asmlinkage long heapsentryk_exit (int a1)
{
	Pid_entry *p_pid_entry = find_pid_entry(original_getpid());
	printk(KERN_INFO "Entered heapsentryk_exit pid:%ld\n",
	       original_getpid());
	// Clean up the hashtable entries.
	free_canaries();

	// Remove the entry of process from linked list.
	if(p_pid_entry){
		list_del(&p_pid_entry->pid_list_head);
	}
	return original_exit(a1);
}

asmlinkage long heapsentryk_clone(unsigned long a1, unsigned long a2,
				  int __user * a3, int __user * a4, int a5)
{

	//printk(KERN_INFO
	//       "Entered heapsentryk_clone() clone clone clone clone clone clone clone\n");
	if (verify_canaries()) {
		heapsentryk_exit(1);
	}
	return original_clone(a1, a2, a3, a4, a5);
}

asmlinkage long heapsentryk_execve(const char __user * a1,
				   const char __user * const __user * a2,
				   const char __user * const __user * a3)
{
	//printk(KERN_INFO "Entered heapsentryk_execve()\n");
	if (verify_canaries()) {
		heapsentryk_exit(1);
	}
	return original_execve(a1, a2, a3);
}

asmlinkage long heapsentryk_open(const char __user * a1, int a2, umode_t a3)
{
	//printk(KERN_INFO "Entered heapsentryk_open()\n");
	if (verify_canaries()) {
		heapsentryk_exit(1);
	}
	return original_open(a1, a2, a3);
}

asmlinkage long heapsentryk_chmod(const char __user * a1, umode_t a2)
{
	/*
	   printk(KERN_INFO
	   "Entered heapsentryk_chmod() chmod chmod chmod chmod chmod\n");
	 */
	if (verify_canaries()) {
		heapsentryk_exit(1);
	}
	return original_chmod(a1, a2);
}

asmlinkage void iterate_pid_list(void)
{
	struct list_head *position = NULL;
	struct list_head *list_iterator = NULL;
	Pid_entry *p_pid_entry = NULL;

	list_for_each_safe(position, list_iterator, &pid_list) {
		p_pid_entry = list_entry(position, Pid_entry, pid_list_head);
		if (p_pid_entry) {
			printk
			    ("p_group_buffer:0x%p p_buffer_count:0x%p pid:%ld hashtable:0x%p \n",
			     p_pid_entry->p_group_buffer,
			     p_pid_entry->p_group_count, p_pid_entry->pid,
			     p_pid_entry->p_process_hashtable);
		}
	}
}

asmlinkage Pid_entry *find_pid_entry(int pid)
{
	Pid_entry *entry = NULL;
	struct list_head *position = NULL;
	struct list_head *list_iterator = NULL;
	Pid_entry *p_pid_entry = NULL;

	list_for_each_safe(position, list_iterator, &pid_list) {
		p_pid_entry = list_entry(position, Pid_entry, pid_list_head);
		if (pid == p_pid_entry->pid) {
			entry = p_pid_entry;
			break;
		}
	}
	return entry;
}

asmlinkage void free_canaries(void)
{
	Pid_entry *p_pid_entry = find_pid_entry(original_getpid());
	if (p_pid_entry) {
		int bucket_index = 0;
		Canary_entry *p_canary_entry = NULL;
		hash_for_each_rcu((*p_pid_entry->p_process_hashtable),
				  bucket_index, p_canary_entry, next) {
			// Removing the entries from hashtable
			hash_del_rcu(&p_canary_entry->next);
			// Free the memory belonging to hashtable entry.
			kfree(p_canary_entry);
		}

	}
}

asmlinkage Canary_entry *list_canaries(struct
				       hlist_head (*hashtable)[1 <<
							       BUCKET_BITS_SIZE])
{
	Canary_entry *entry = NULL;
	int bucket_index = 0;
	Canary_entry *p_canary_entry = NULL;
	hash_for_each_rcu((*hashtable), bucket_index, p_canary_entry, next) {
		printk(KERN_INFO
		       "canary_location=%d canary_value=%d deref=%d is in bucket %d\n",
		       p_canary_entry->canary_location,
		       p_canary_entry->canary_value,
		       *((size_t *) p_canary_entry->canary_location),
		       bucket_index);
	}
	return entry;
}

asmlinkage Canary_entry *find_hashtable_entry(size_t canary_location, struct
					      hlist_head(*hashtable)[1 <<
								     BUCKET_BITS_SIZE])
{
	Canary_entry *entry = NULL;
	int bucket_index = 0;
	Pid_entry *p_pid_entry = find_pid_entry(original_getpid());

	Canary_entry *p_canary_entry = NULL;

	hash_for_each_rcu((*p_pid_entry->p_process_hashtable), bucket_index,
			  p_canary_entry, next) {
		if (p_canary_entry->canary_value ==
		    *((size_t *) p_canary_entry->canary_location)) {
			entry = p_canary_entry;
			break;
		}
	}
	return entry;
}

asmlinkage int verify_canaries(void)
{
	Pid_entry *p_pid_entry = find_pid_entry(original_getpid());
	if (p_pid_entry) {
		int bucket_index = 0;
		Canary_entry *p_canary_entry = NULL;
		hash_for_each_rcu((*(p_pid_entry->p_process_hashtable)),
				  bucket_index, p_canary_entry, next) {
			if (p_canary_entry->canary_value !=
			    *((size_t *) p_canary_entry->canary_location)) {
				printk(KERN_INFO
				       "Canary verification failed. Forcing exit to ensure security!\n");
				return 1;
			}
		}
	}
	return 0;
}

asmlinkage size_t sys_heapsentryk_canary_init(size_t not_used, size_t v2,
					      size_t v3)
{
	size_t *p_group_buffer = (size_t *) v2;
	size_t *p_group_count = (size_t *) v3;
	Pid_entry *p_pid_entry =
	    (Pid_entry *) kmalloc(sizeof(Pid_entry), GFP_KERNEL);
	struct hlist_head (*buckets)[1 << BUCKET_BITS_SIZE] =
	    (struct hlist_head(*)[1 << BUCKET_BITS_SIZE])
	    kmalloc(sizeof
		    (struct hlist_head) *(1 << BUCKET_BITS_SIZE), GFP_KERNEL);
	// Braces are added to void bugs that could arise by macro substitutions.
	// Since there is an asterisk in the argument, if the macro places the
	// argument as a suffix, there will be a problem.
	hash_init((*buckets));

	// TODO: perform input validation:w
	printk("canary_init called for pid:%ld\n", original_getpid());

	p_pid_entry->p_group_buffer = p_group_buffer;
	p_pid_entry->p_group_count = p_group_count;
	p_pid_entry->pid = original_getpid();
	p_pid_entry->p_process_hashtable = buckets;
	INIT_LIST_HEAD(&p_pid_entry->pid_list_head);

	list_add(&p_pid_entry->pid_list_head, &pid_list);

	//Testing
	printk("testing init entry pid\n");
	iterate_pid_list();
	printk("testing init entry pid  ended\n");
	return 0;
}

// System call which receives the canary information from heapsentryu
// and stores it in its symbol table. This information is used by
// high-risk calls to verify the canaries.
asmlinkage size_t sys_heapsentryk_canary(void)
{
	struct hlist_head (*p_hash)[1 << BUCKET_BITS_SIZE] = NULL;

	Pid_entry *p_pid_entry = NULL;

	/*
	   printk("buf[%d][0]:[%p] buf[%d][1]:[%d] deref:[%d]\n", i,
	   (void *)*(p_group_buffer + i * 2), i,
	   *(p_group_buffer + i * 2 + 1),
	   *((size_t *) * (p_group_buffer + i * 2)));
	 */

	// Checking if there is an entry in the linked list corresponding to the current process.
	p_pid_entry = find_pid_entry(original_getpid());

	if (p_pid_entry) {
		size_t i = 0;
		printk
		    ("heapsentryk:received: p_group_buffer:[%p] p_group_count:[%p] \n",
		     p_pid_entry->p_group_buffer, p_pid_entry->p_group_count);
		printk("heapsentryk: dereferencing p_group_count:[%d]\n",
		       *p_pid_entry->p_group_count);

		printk("Found pid_entry:%p found for pid:%ld\n", p_pid_entry,
		       original_getpid());

		// Adding canaries to hashtable.
		for (i = 0; i < *(p_pid_entry->p_group_count); i++) {
			Canary_entry *entry =
			    (Canary_entry *) kmalloc(sizeof(Canary_entry),
						     GFP_KERNEL);
			memset((void *)entry, 0, sizeof(Canary_entry));
			entry->canary_location =
			    *(p_pid_entry->p_group_buffer + i * 2);
			entry->canary_value =
			    *((size_t *) *
			      (p_pid_entry->p_group_buffer + i * 2));
			hash_add_rcu((*p_pid_entry->p_process_hashtable),
				     &entry->next, entry->canary_location);
		}
		p_hash = p_pid_entry->p_process_hashtable;
		list_canaries(p_hash);
	} else {
		printk
		    ("no entry found in linked list for this process: pid:%ld\n",
		     original_getpid());
	}
	return 0;
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
	original_execve = sys_call_table[__NR_execve];
	original_open = sys_call_table[__NR_open];
	original_chmod = sys_call_table[__NR_chmod];
	original_getpid = sys_call_table[__NR_getpid];
	original_exit = sys_call_table[__NR_exit_group];
	original_clone = sys_call_table[__NR_clone];
	original_fchmod = sys_call_table[__NR_fchmod];

	// Substituting the system calls with heapsentryk's calls.
	// In these substituted function, decision can be taken regarding further
	// actions. Either exiting the process or letting the request through to
	// original syscalls.
	sys_call_table[__NR_open] = heapsentryk_open;
	sys_call_table[__NR_execve] = heapsentryk_execve;
	sys_call_table[__NR_chmod] = heapsentryk_chmod;
	sys_call_table[__NR_clone] = heapsentryk_clone;
	sys_call_table[__NR_exit_group] = heapsentryk_exit;
	sys_call_table[__NR_fchmod] = heapsentryk_fchmod;

	// Setting HeapSentryu's sys_canary() to sys_call_table to the configured index.
	sys_call_table[SYS_CALL_NUMBER] = sys_heapsentryk_canary;

	// Setting HeapSentryu's sys_canary_init() to sys_call_table to the configured index.
	sys_call_table[SYS_CANARY_INIT_NUMBER] = sys_heapsentryk_canary_init;

	return 0;
}

// Exit function of this kernel module. This gets executed when this module
// is unloaded from kernel.
static void __exit mod_exit_func(void)
{
	// Restoring the original addresses of the system calls in the table.
	sys_call_table[__NR_execve] = original_execve;
	sys_call_table[__NR_open] = original_open;
	sys_call_table[__NR_chmod] = original_chmod;
	sys_call_table[__NR_exit_group] = original_exit;
	sys_call_table[__NR_clone] = original_clone;
	sys_call_table[__NR_fchmod] = original_fchmod;

	sys_call_table[SYS_CALL_NUMBER] = 0;
	sys_call_table[SYS_CANARY_INIT_NUMBER] = 0;
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
