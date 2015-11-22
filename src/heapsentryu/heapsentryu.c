#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define __USE_GNU
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

// These are the real libc functions
extern void *__libc_realloc(void *ptr, size_t size);
extern void *__libc_malloc(size_t size);
extern void __libc_free(void *ptr);

typedef struct malloc_info {
	size_t *malloc_location;
	size_t *canary_location;
	size_t canary;
} Malloc_info;

pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

Malloc_info *p_group_buffer = NULL;
size_t group_count = 0;

// The function which invokes the HeapSentry's system call.
size_t sys_canary();

int get_free_index();

// This system call informs kernel the address of group buffer and group count variable.
size_t sys_canary_init();

size_t sys_canary_free(void *obj);

// Initializes the random number generator
void heapsentryu_init();

int get_free_index()
{
	int i = 0;
	for (i = 0; i < CANARY_GROUP_SIZE; i++) {
		if (p_group_buffer[i].malloc_location == 0) {
			return i;
		}
	}
	return -1;
}

// This function generates a random canary and writes it after the requested
// malloc block and stores these info into group_buffer.
// group_buffer will be pushed to kernel when count reaches CANARY_GROUP_SIZE
void push_canary(char *obj, size_t size)
{
	pthread_mutex_lock(&mutex);

	// Determining the canary location to store the random number and casting it to int*.
	size_t *canary_location = (size_t *) (obj + size);

	// Generating a random number and casting it to size of int.
	int canary = (int)rand();

	// Saving the canary at its designated location.
	*canary_location = canary;

	int free_idx = get_free_index();
	if (free_idx >= 0) {
		p_group_buffer[free_idx].malloc_location = (size_t *) obj;
		p_group_buffer[free_idx].canary_location = canary_location;
		p_group_buffer[free_idx].canary = canary;

		/*
		   printf("free_index:%d obj:%p can_loc:%p can:%d\n", free_idx,
		   p_group_buffer[group_count].malloc_location,
		   p_group_buffer[group_count].canary_location,
		   p_group_buffer[group_count].canary);
		 */
		group_count++;

		if (group_count == CANARY_GROUP_SIZE) {
			sys_canary();
		}
	}

	pthread_mutex_unlock(&mutex);
}

// This function removes canary from group_buffer and/or kernel.
void free_canary(void *obj)
{
	pthread_mutex_lock(&mutex);

	// Do not proceed if canary information is being communicated to kernel.
	if (obj != NULL && p_group_buffer != NULL) {
		int i = 0;
		for (i = 0; i < CANARY_GROUP_SIZE && group_count; i++) {
			if (p_group_buffer[i].malloc_location == obj) {
				//printf("Free found obj:%p in buffer at index:%d\n", obj, i);
				p_group_buffer[i].malloc_location = NULL;
				p_group_buffer[i].canary_location = NULL;
				p_group_buffer[i].canary = 0;
				group_count--;
				break;
			}
		}

		sys_canary_free(obj);
	}

	pthread_mutex_unlock(&mutex);
}

// If this library is preloaded before any binary execution, then
// this malloc() will be invoked, instead of stdlib malloc().
// 
// After performing the actions to be taken in this method, 
// real malloc is called using __libc_malloc.
void *malloc(size_t size)
{
	// This happens once per library load.
	// Setting the seed for random number generation,
	// informing kernel about group_buffer & group_count address.
	heapsentryu_init();

	// sizeof(int) depends on the compiler and not the hardware.
	// So, it is not good to assume it to be 4 or 8 or whatever.
	size_t modified_size = size + sizeof(int);

	// Requesting malloc to allocate space for an extra integer along with `size` bytes.
	char *obj = (char *)__libc_malloc(modified_size);

	push_canary(obj, size);

	return obj;
}

// If this library is preloaded before any binary execution, then
// this free will be invoked instead of stdlib free.
//
// After performing the actions to be taken in this method, 
// real free is called using __libc_free.
void free(void *obj)
{
	// This happens once per library load.
	// Setting the seed for random number generation,
	// informing kernel about group_buffer & group_count address.
	heapsentryu_init();

	free_canary(obj);

	__libc_free(obj);
}

// If this library is preloaded before any binary execution, then
// this calloc will be invoked instead of stdlib calloc.
//
// After performing the actions to be taken in this method, 
// we allocate memory using __libc_malloc.
void *calloc(size_t nmemb, size_t size)
{
	// This happens once per library load.
	// Setting the seed for random number generation,
	// informing kernel about group_buffer & group_count address.
	heapsentryu_init();

	size_t actual_size = size * nmemb;

	// sizeof(int) depends on the compiler and not the hardware.
	// So, it is not good to assume it to be 4 or 8 or whatever.
	size_t modified_size = actual_size + sizeof(int);

	// Note: we are not calling real calloc, instead using malloc only.
	// Requesting malloc to allocate space for an extra integer along with 
	// `actual_size` bytes.
	char *obj = (char *)__libc_malloc(modified_size);
	memset(obj, 0, actual_size);

	push_canary(obj, actual_size);

	return obj;
}

// If this library is preloaded before any binary execution, then
// this realloc will be invoked instead of stdlib realloc.
//
// After performing the actions to be taken in this method, 
// real realloc is called using __libc_realloc.
void *realloc(void *obj, size_t size)
{
	// This happens once per library load.
	// Setting the seed for random number generation,
	// informing kernel about group_buffer & group_count address.
	heapsentryu_init();

	// Remove this entry, we will add new one with new address
	free_canary(obj);

	// sizeof(int) depends on the compiler and not the hardware.
	// So, it is not good to assume it to be 4 or 8 or whatever.
	size_t modified_size = size + sizeof(int);

	obj = __libc_realloc(obj, modified_size);

	push_canary(obj, size);

	return obj;
}

size_t sys_canary_init()
{
	size_t r = -1;
	size_t n = (size_t) SYS_CANARY_INIT_NUMBER;
	__asm__ __volatile__("int $0x80":"=a"(r):"a"(n),
			     "D"((size_t) p_group_buffer), "S"(n),
			     "d"((size_t) & group_count):"cc", "memory");
	return r;
}

size_t sys_canary_free(void *obj)
{
	size_t r = -1;
	size_t n = (size_t) SYS_CANARY_FREE_NUMBER;
	__asm__ __volatile__("int $0x80":"=a"(r):"a"(n),
			     "D"((size_t) obj), "S"(n),
			     "d"((size_t) obj):"cc", "memory");
	return r;
}

size_t sys_canary()
{

	size_t r = -1;
	size_t n = (size_t) SYS_CALL_NUMBER;
	__asm__ __volatile__("int $0x80":"=a"(r):"a"(n):"cc", "memory");
	return r;
}

// This function initializes the random number generator only ONCE per process
// invocation. If we enter this library from another process or successive invocation
// of same process, init_done will be zero again and srand init will happen again.
// This makes sure we have fresh random numbers for every invocation of a process
// Warning: There is still a window of upto 1sec where the same seq of random numbers 
// are produced across process invocations. Cos time(NULL) will be the same within a
// second.
void heapsentryu_init(void)
{
	static int init_done = 0;

	pthread_mutex_lock(&mutex);

	if (!init_done) {
		init_done = 1;

		int actual_size = CANARY_GROUP_SIZE * sizeof(Malloc_info);
		long PAGE_SIZE = sysconf(_SC_PAGESIZE);
		int no_of_pages = actual_size / PAGE_SIZE;
		if (actual_size % PAGE_SIZE != 0) {
			no_of_pages++;
		}
		char *buffer =
		    (char *)__libc_malloc(PAGE_SIZE * (no_of_pages + 3));
		buffer =
		    (char *)(((int)buffer + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
		// Guarding 1 page at the head of the buffer
		if (mprotect(buffer, PAGE_SIZE, PROT_NONE)) {
			printf
			    ("mprotect() failed while safeguarding head of buffer: err:%d\n",
			     errno);
		}
		// Guarding 1 page at the tail of the buffer
		if (mprotect
		    (buffer + PAGE_SIZE * (no_of_pages + 1), PAGE_SIZE,
		     PROT_NONE)) {
			printf
			    ("mprotect() failed while safeguarding tail of buffer: err:%d\n",
			     errno);
		}

		p_group_buffer = (Malloc_info *) (buffer + PAGE_SIZE);

		sys_canary_init();
		srand(time(NULL));
	}

	pthread_mutex_unlock(&mutex);
}
