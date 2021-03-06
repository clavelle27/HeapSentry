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
#define BUCKET_BITS_SIZE 10

// Define a linked list that holds information that connects PID with a process specific hashtable
LIST_HEAD(pid_list);

typedef struct malloc_info {
	size_t *malloc_location;
	size_t *canary_location;
	size_t canary;
} Malloc_info;

// The object that the link list holds.
typedef struct pid_entry {
	Malloc_info *p_group_buffer;
	size_t *p_group_count;
	long pid;
	struct hlist_head (*p_process_hashtable)[1 << BUCKET_BITS_SIZE];
	struct list_head pid_list_head;
	int total_canaries;
} Pid_entry;

// The object identified by a key(canary_location) in hashtable which holds canary information.
typedef struct canary_entry {
	Malloc_info minfo;
	struct hlist_node next;
} Canary_entry;

asmlinkage long (*original_open_by_handle_at) (int a1, void *a2, int a3);
asmlinkage long (*original_openat) (int a1, const char __user * a2, int a3,
				    umode_t a4);
asmlinkage long (*original_creat) (const char __user * a1, umode_t a2);
asmlinkage long (*original_fork) (void);
asmlinkage long (*original_vfork) (void);
asmlinkage long (*original_fchmodat) (int a1, const char __user * a2,
				      umode_t a3);
asmlinkage long (*original_fchmod) (unsigned int a1, umode_t a2);
asmlinkage long (*original_chmod) (const char __user * a1, umode_t a2);
asmlinkage long (*original_execve) (const char __user * a1,
				    const char __user * const __user * a2,
				    const char __user * const __user * a3);
asmlinkage long (*original_open) (const char __user * a1, int a2, umode_t a3);
asmlinkage long (*original_exit) (int a1);
asmlinkage long (*original_exit_group) (int a1);
asmlinkage long (*original_getpid) (void);
asmlinkage long (*original_clone) (unsigned long a1, unsigned long a2,
				   int __user * a3, int __user * a4, int a5);

asmlinkage long (*original_read) (unsigned int a1, char __user * a2, size_t a3);
asmlinkage long (*original_write) (unsigned int a1, const char __user * a2,
				   size_t a3);
asmlinkage long (*original_mount) (char __user * a1, char __user * a2,
				   char __user * a3, unsigned long a4,
				   void __user * a5);

asmlinkage long heapsentryk_open_by_handle_at(int a1, void *a2, int a3);
asmlinkage long heapsentryk_openat(int a1, const char __user * a2, int a3,
				   umode_t a4);
asmlinkage long heapsentryk_creat(const char __user * a1, umode_t a2);
asmlinkage long heapsentryk_fork(void);
asmlinkage long heapsentryk_vfork(void);
asmlinkage long heapsentryk_fchmodat(int a1, const char __user * a2,
				     umode_t a3);
asmlinkage long heapsentryk_fchmod(unsigned int a1, umode_t a2);
asmlinkage long heapsentryk_chmod(const char __user * a1, umode_t a2);
asmlinkage long heapsentryk_execve(const char __user * a1,
				   const char __user * const __user * a2,
				   const char __user * const __user * a3);
asmlinkage long heapsentryk_open(const char __user * a1, int a2, umode_t a3);
asmlinkage long heapsentryk_getpid(void);
asmlinkage long heapsentryk_clone(unsigned long a1, unsigned long a2,
				  int __user * a3, int __user * a4, int a5);
asmlinkage long heapsentryk_exit(int a1);
asmlinkage long heapsentryk_exit_group(int a1);
asmlinkage long heapsentryk_read(unsigned int a1, char __user * a2, size_t a3);
asmlinkage long heapsentryk_write(unsigned int a1, const char __user * a2,
				  size_t a3);
asmlinkage long heapsentryk_mount(char __user * a1, char __user * a2,
				  char __user * a3, unsigned long a4,
				  void __user * a5);

void set_read_write(long unsigned int _addr);
asmlinkage int cleanup(void);
asmlinkage int remove_hashtable_entry(size_t * malloc_location);
asmlinkage int copy_canaries(size_t from_pid, size_t to_pid);
asmlinkage void free_canaries(void);
asmlinkage Pid_entry *find_pid_entry(int pid);
asmlinkage size_t sys_heapsentryk_canary(void);
asmlinkage size_t canary_init(size_t pid, Malloc_info * p_group_buffer,
			      size_t * p_group_count);
asmlinkage size_t sys_heapsentryk_canary_init(size_t not_used, size_t v2,
					      size_t v3);
asmlinkage size_t sys_heapsentryk_canary_free(size_t not_used, size_t v2,
					      size_t v3);
asmlinkage void iterate_pid_list(void);
asmlinkage int pull_and_verify_canaries(int percentage, char *called_from);

asmlinkage long heapsentryk_mount(char __user * a1, char __user * a2,
				  char __user * a3, unsigned long a4,
				  void __user * a5)
{
	//printk(KERN_INFO "Entered heapsentryk_mount pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(MEDIUM_RISK_PERCENTAGE, "mount")) {
		heapsentryk_exit_group(1);
	}
	return original_mount(a1, a2, a3, a4, a5);
}

asmlinkage long heapsentryk_read(unsigned int a1, char __user * a2, size_t a3)
{
	//printk(KERN_INFO "Entered heapsentryk_read pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(MEDIUM_RISK_PERCENTAGE, "read")) {
		heapsentryk_exit_group(1);
	}
	return original_read(a1, a2, a3);
}

asmlinkage long heapsentryk_write(unsigned int a1, const char __user * a2,
				  size_t a3)
{
	//printk(KERN_INFO "Entered heapsentryk_write pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(MEDIUM_RISK_PERCENTAGE, "write")) {
		heapsentryk_exit_group(1);
	}
	return original_write(a1, a2, a3);
}

asmlinkage long heapsentryk_open_by_handle_at(int a1, void *a2, int a3)
{
	//printk(KERN_INFO "Entered heapsentryk_open_by_handle_at pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "open_by_handle_at")) {
		heapsentryk_exit_group(1);
	}
	return original_open_by_handle_at(a1, a2, a3);
}

asmlinkage long heapsentryk_openat(int a1, const char __user * a2, int a3,
				   umode_t a4)
{
	//printk(KERN_INFO "Entered heapsentryk_openat pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "openat")) {
		heapsentryk_exit_group(1);
	}
	return original_openat(a1, a2, a3, a4);
}

asmlinkage long heapsentryk_creat(const char __user * a1, umode_t a2)
{
	//printk(KERN_INFO "Entered heapsentryk_creat pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "creat")) {
		heapsentryk_exit_group(1);
	}
	return original_creat(a1, a2);
}

asmlinkage long heapsentryk_vfork(void)
{
	//printk(KERN_INFO "Entered heapsentryk_vfork pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "vfork")) {
		heapsentryk_exit_group(1);
	}
	return original_vfork();
}

asmlinkage long heapsentryk_fork(void)
{
	//printk(KERN_INFO "Entered heapsentryk_fork pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "fork")) {
		heapsentryk_exit_group(1);
	}
	return original_fork();
}

asmlinkage long heapsentryk_fchmodat(int a1, const char __user * a2, umode_t a3)
{
	//printk(KERN_INFO "Entered heapsentryk_fchmodat pid:%ld\n",
	//      original_getpid());
	if (pull_and_verify_canaries(100, "fchmodat")) {
		heapsentryk_exit_group(1);
	}
	return original_fchmodat(a1, a2, a3);
}

asmlinkage long heapsentryk_fchmod(unsigned int a1, umode_t a2)
{
	//printk(KERN_INFO "Entered heapsentryk_fchmod pid:%ld\n",
	//      original_getpid());
	if (pull_and_verify_canaries(100, "fchmod")) {
		heapsentryk_exit_group(1);
	}
	return original_fchmod(a1, a2);
}

asmlinkage long heapsentryk_clone(unsigned long a1, unsigned long a2,
				  int __user * a3, int __user * a4, int a5)
{

	size_t pid = 0;
	size_t current_pid = original_getpid();
	//printk(KERN_INFO
	//       "Entered heapsentryk_clone pid:%ld\n", original_getpid());
	if (pull_and_verify_canaries(100, "clone")) {
		heapsentryk_exit_group(1);
	}
	if (0 == (pid = original_clone(a1, a2, a3, a4, a5))) {
		return 0;
	} else {
		Pid_entry *p_pid_entry = NULL;
		p_pid_entry = find_pid_entry(current_pid);
		if (p_pid_entry) {
			canary_init(pid, p_pid_entry->p_group_buffer,
				    p_pid_entry->p_group_count);
			copy_canaries(current_pid, pid);
			iterate_pid_list();
		}
		return pid;
	}
}

asmlinkage long heapsentryk_open(const char __user * a1, int a2, umode_t a3)
{
	//printk(KERN_INFO "Entered heapsentryk_open pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "open")) {
		heapsentryk_exit_group(1);
	}
	return original_open(a1, a2, a3);
}

asmlinkage long heapsentryk_chmod(const char __user * a1, umode_t a2)
{
	//printk(KERN_INFO "Entered heapsentryk_chmod pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "chmod")) {
		heapsentryk_exit_group(1);
	}
	return original_chmod(a1, a2);
}

asmlinkage long heapsentryk_exit(int a1)
{
	//printk(KERN_INFO "Entered heapsentryk_exit pid:%ld\n",
	//       original_getpid());
	return original_exit(a1);
}

asmlinkage long heapsentryk_exit_group(int a1)
{
	//printk(KERN_INFO "Entered heapsentryk_exit_group pid:%ld\n",
	//       original_getpid());

	cleanup();
	return original_exit_group(a1);
}

asmlinkage long heapsentryk_execve(const char __user * a1,
				   const char __user * const __user * a2,
				   const char __user * const __user * a3)
{
	//printk(KERN_INFO "Entered heapsentryk_execve pid:%ld\n",
	//       original_getpid());
	if (pull_and_verify_canaries(100, "execve")) {
		heapsentryk_exit_group(1);
	}
	cleanup();

	return original_execve(a1, a2, a3);
}

asmlinkage int cleanup(void)
{

	Pid_entry *p_pid_entry = find_pid_entry(original_getpid());
	free_canaries();
	if (p_pid_entry) {
		// Remove the element representing the process from linked list
		list_del(&p_pid_entry->pid_list_head);

		// Free the memory allocated for the element
		kfree(p_pid_entry);

		// Avoid dangling pointer
		p_pid_entry = NULL;
	}
	return 0;
}

asmlinkage void iterate_pid_list(void)
{
	struct list_head *position = NULL;
	struct list_head *list_iterator = NULL;
	Pid_entry *p_pid_entry = NULL;

	list_for_each_safe(position, list_iterator, &pid_list) {
		p_pid_entry = list_entry(position, Pid_entry, pid_list_head);
		if (p_pid_entry) {
			//printk
			//    ("p_group_buffer:0x%p p_buffer_count:0x%p pid:%ld hashtable:0x%p \n",
			//     p_pid_entry->p_group_buffer,
			//     p_pid_entry->p_group_count, p_pid_entry->pid,
			//     p_pid_entry->p_process_hashtable);
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
			p_pid_entry->total_canaries--;

			// Free the memory belonging to hashtable entry.
			kfree(p_canary_entry);
		}

	}
}

asmlinkage int copy_canaries(size_t from_pid, size_t to_pid)
{
	int bucket_index = 0;
	Canary_entry *p_canary_entry = NULL;
	Pid_entry *from_pid_entry = find_pid_entry(from_pid);
	Pid_entry *to_pid_entry = find_pid_entry(to_pid);
	//printk("hey hey hey!!!\n");
	hash_for_each_rcu((*from_pid_entry->p_process_hashtable), bucket_index,
			  p_canary_entry, next) {
		Canary_entry *entry = (Canary_entry *)
		    kmalloc(sizeof(Canary_entry),
			    GFP_KERNEL);
		//printk(KERN_INFO
		//       "Copying %d->%d, malloc_location=%p canary_location=%p canary=%d deref=%d is in bucket %d\n",
		//       from_pid, to_pid,
		//       p_canary_entry->minfo.malloc_location,
		//       p_canary_entry->minfo.canary_location,
		//       p_canary_entry->minfo.canary,
		//       *((size_t *) p_canary_entry->minfo.canary_location),
		//       bucket_index);
		memset((void *)entry, 0, sizeof(Canary_entry));
		entry->minfo.malloc_location =
		    p_canary_entry->minfo.malloc_location;
		entry->minfo.canary_location =
		    p_canary_entry->minfo.canary_location;
		entry->minfo.canary = p_canary_entry->minfo.canary;

		hash_add_rcu((*to_pid_entry->p_process_hashtable),
			     &entry->next,
			     ((size_t) entry->minfo.malloc_location));
	}

	to_pid_entry->total_canaries = from_pid_entry->total_canaries;

	return 0;
}

asmlinkage int remove_hashtable_entry(size_t * malloc_location)
{
	int bucket_index = 0;
	Pid_entry *p_pid_entry = find_pid_entry(original_getpid());
	Canary_entry *p_canary_entry = NULL;

	if (!p_pid_entry) {
		return 0;
	}

	hash_for_each_rcu((*p_pid_entry->p_process_hashtable), bucket_index,
			  p_canary_entry, next) {
		if (p_canary_entry->minfo.malloc_location == malloc_location) {
			//printk("Heapsentryk: removing obj:%p\n",
			//       p_canary_entry->minfo.malloc_location);
			hash_del_rcu(&p_canary_entry->next);
			p_pid_entry->total_canaries--;
			kfree(p_canary_entry);
			break;
		}
	}
	return 0;
}

asmlinkage int pull_and_verify_canaries(int percentage, char *called_from)
{
	int to_be_verified = 0;
	Pid_entry *p_pid_entry = find_pid_entry(original_getpid());
	if (p_pid_entry) {
		int bucket_index = 0;
		Canary_entry *p_canary_entry = NULL;
		if (*p_pid_entry->p_group_count) {
			//printk
			//    ("PID:%ld [%s] invoking pull to retrieve [%d] items.\n",
			//     original_getpid(), called_from,
			//     *p_pid_entry->p_group_count);

			//Pull the canary information from user space if they are present
			sys_heapsentryk_canary();
		}

		to_be_verified =
		    (p_pid_entry->total_canaries * percentage) / 100;

		hash_for_each_rcu((*(p_pid_entry->p_process_hashtable)),
				  bucket_index, p_canary_entry, next) {
			if (to_be_verified <= 0) {
				break;
			}
			if (p_canary_entry->minfo.canary !=
			    *((size_t *) p_canary_entry->
			      minfo.canary_location)) {
				printk(KERN_INFO
				       "Canary verification failed for PID:%ld. Forcing exit to ensure security!\n",original_getpid());
				return 1;
			}
			to_be_verified--;
		}
	}
	return 0;
}

asmlinkage size_t sys_heapsentryk_canary_free(size_t not_used, size_t v2,
					      size_t not_used1)
{
	size_t *obj = (size_t *) v2;
	//printk(KERN_INFO "sys_heapsentryk_canary_free entered: obj:%p\n", obj);
	remove_hashtable_entry(obj);
	return 0;
}

asmlinkage size_t canary_init(size_t pid, Malloc_info * p_group_buffer,
			      size_t * p_group_count)
{
	Pid_entry *p_pid_entry = find_pid_entry(pid);
	if (!p_pid_entry) {
		struct hlist_head (*buckets)[1 << BUCKET_BITS_SIZE] =
		    (struct hlist_head(*)[1 << BUCKET_BITS_SIZE])
		    kmalloc(sizeof
			    (struct hlist_head) * (1 << BUCKET_BITS_SIZE),
			    GFP_KERNEL);
		p_pid_entry =
		    (Pid_entry *) kmalloc(sizeof(Pid_entry), GFP_KERNEL);
		// Braces are added to void bugs that could arise by macro substitutions.
		// Since there is an asterisk in the argument, if the macro places the
		// argument as a suffix, there will be a problem.
		hash_init((*buckets));

		printk("CANARY_INIT: PID:%d\n", pid);

		p_pid_entry->p_group_buffer = p_group_buffer;
		p_pid_entry->p_group_count = p_group_count;
		p_pid_entry->pid = pid;
		p_pid_entry->p_process_hashtable = buckets;
		p_pid_entry->total_canaries = 0;
		INIT_LIST_HEAD(&p_pid_entry->pid_list_head);
		list_add(&p_pid_entry->pid_list_head, &pid_list);
	} else {
		printk(KERN_INFO "CANARY_TABLE ready for PID:%d\n", pid);
	}
	return 0;
}

asmlinkage size_t sys_heapsentryk_canary_init(size_t not_used, size_t v2,
					      size_t v3)
{
	return canary_init(original_getpid(), (Malloc_info *) v2,
			   (size_t *) v3);
}

// System call which receives the canary information from heapsentryu
// and stores it in its symbol table. This information is used by
// high-risk calls to verify the canaries.
asmlinkage size_t sys_heapsentryk_canary(void)
{
	struct hlist_head (*p_hash)[1 << BUCKET_BITS_SIZE] = NULL;

	Pid_entry *p_pid_entry = NULL;

	// Checking if there is an entry in the linked list corresponding to the current process.
	p_pid_entry = find_pid_entry(original_getpid());

	if (p_pid_entry) {
		size_t i = 0;
		/*
		   printk
		   ("heapsentryk: p_group_buffer:[%p] group_count:[%d] \n",
		   p_pid_entry->p_group_buffer,
		   *p_pid_entry->p_group_count);
		 */
		// Adding canaries to hashtable.
		for (i = 0;
		     i < CANARY_GROUP_SIZE && *p_pid_entry->p_group_count;
		     i++) {
			if (p_pid_entry->p_group_buffer[i].malloc_location !=
			    NULL) {
				Canary_entry *entry = (Canary_entry *)
				    kmalloc(sizeof(Canary_entry),
					    GFP_KERNEL);
				memset((void *)entry, 0, sizeof(Canary_entry));
				entry->minfo.malloc_location =
				    p_pid_entry->p_group_buffer[i].
				    malloc_location;
				entry->minfo.canary_location =
				    p_pid_entry->p_group_buffer[i].
				    canary_location;
				entry->minfo.canary =
				    p_pid_entry->p_group_buffer[i].canary;

				hash_add_rcu((*p_pid_entry->
					      p_process_hashtable),
					     &entry->next,
					     ((size_t) entry->minfo.
					      malloc_location));

				p_pid_entry->total_canaries++;

				p_pid_entry->p_group_buffer[i].malloc_location =
				    NULL;
				p_pid_entry->p_group_buffer[i].canary_location =
				    NULL;
				p_pid_entry->p_group_buffer[i].canary = 0;
				(*p_pid_entry->p_group_count)--;
			}
		}
		p_hash = p_pid_entry->p_process_hashtable;
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
	original_exit = sys_call_table[__NR_exit];
	original_exit_group = sys_call_table[__NR_exit_group];
	original_clone = sys_call_table[__NR_clone];
	original_fchmod = sys_call_table[__NR_fchmod];
	original_fchmodat = sys_call_table[__NR_fchmodat];
	original_fork = sys_call_table[__NR_fork];
	original_vfork = sys_call_table[__NR_vfork];
	original_openat = sys_call_table[__NR_openat];
	original_open_by_handle_at = sys_call_table[__NR_open_by_handle_at];
	original_read = sys_call_table[__NR_read];
	original_write = sys_call_table[__NR_write];
	original_mount = sys_call_table[__NR_mount];

	// Substituting the system calls with heapsentryk's calls.
	// In these substituted function, decision can be taken regarding further
	// actions. Either exiting the process or letting the request through to
	// original syscalls.
	sys_call_table[__NR_open] = heapsentryk_open;
	sys_call_table[__NR_execve] = heapsentryk_execve;
	sys_call_table[__NR_chmod] = heapsentryk_chmod;
	sys_call_table[__NR_clone] = heapsentryk_clone;
	sys_call_table[__NR_exit_group] = heapsentryk_exit_group;
	sys_call_table[__NR_exit] = heapsentryk_exit;
	sys_call_table[__NR_fchmod] = heapsentryk_fchmod;
	sys_call_table[__NR_fchmodat] = heapsentryk_fchmodat;
	sys_call_table[__NR_fork] = heapsentryk_fork;
	sys_call_table[__NR_vfork] = heapsentryk_vfork;
	sys_call_table[__NR_openat] = heapsentryk_openat;
	sys_call_table[__NR_open_by_handle_at] = heapsentryk_open_by_handle_at;
	sys_call_table[__NR_read] = heapsentryk_read;
	sys_call_table[__NR_write] = heapsentryk_write;
	sys_call_table[__NR_mount] = heapsentryk_mount;

	// Setting HeapSentryu's sys_canary() to sys_call_table to the configured index.
	sys_call_table[SYS_CANARY_NUMBER] = sys_heapsentryk_canary;

	// Setting HeapSentryu's sys_canary_init() to sys_call_table to the configured index.
	sys_call_table[SYS_CANARY_INIT_NUMBER] = sys_heapsentryk_canary_init;

	sys_call_table[SYS_CANARY_FREE_NUMBER] = sys_heapsentryk_canary_free;

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
	sys_call_table[__NR_exit_group] = original_exit_group;
	sys_call_table[__NR_exit] = original_exit;
	sys_call_table[__NR_clone] = original_clone;
	sys_call_table[__NR_fchmod] = original_fchmod;
	sys_call_table[__NR_fchmodat] = original_fchmodat;
	sys_call_table[__NR_fork] = original_fork;
	sys_call_table[__NR_vfork] = original_vfork;
	sys_call_table[__NR_openat] = original_openat;
	sys_call_table[__NR_open_by_handle_at] = original_open_by_handle_at;
	sys_call_table[__NR_read] = original_read;
	sys_call_table[__NR_write] = original_write;
	sys_call_table[__NR_mount] = original_mount;

	sys_call_table[SYS_CANARY_NUMBER] = 0;
	sys_call_table[SYS_CANARY_INIT_NUMBER] = 0;
	sys_call_table[SYS_CANARY_FREE_NUMBER] = 0;

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
