#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define __USE_GNU
#include <dlfcn.h>

size_t *p_group_buffer;
size_t group_count = 0;
// These are the function pointers used to hold the address of 
// actual system calls. The syntax of these function pointers
// should precisely match with the syntax of real system calls.
typedef void *(*real_malloc) (size_t size);
typedef void (*real_free) (void *ptr);

// The function which invokes the HeapSentry's system call.
size_t sys_canary();

// This system call informs kernel the address of group buffer and group count variable.
size_t sys_canary_init();

// Initializes the random number generator
void heapsentryu_init();

// If this library is preloaded before any binary execution, then
// this malloc() will be invoked, instead of stdlib malloc().
// 
// After performing the actions to be taken in this method, the address
// of real malloc() is determined and invoked.
//
// Address of real malloc() is determined using libdl. RTLD_NEXT
// indicates dlsym() to find for the next symbol that goes by the
// provided name.
void *malloc(size_t size)
{
	// Determine the function pointer of libc malloc().
	real_malloc rmalloc = (real_malloc) dlsym(RTLD_NEXT, "malloc");

	// TODO: Fill up an array (probably) of size configured by user and pass it
	// to kernel when it gets filled.
	if (!p_group_buffer) {
		p_group_buffer =
		    (size_t *) rmalloc(CANARY_GROUP_SIZE * 2 * sizeof(size_t));
	}


	// sizeof(int) depends on the compiler and not the hardware.
	// So, it is not good to assume it to be 4 or 8 or whatever.
	size_t modified_size = size + sizeof(int);

	// Requesting malloc to allocate space for an extra integer along with `size` bytes.
	char *obj = (char *)rmalloc(modified_size);

	// Determining the canary location to store the random number and casting it to int*.
	int *canary_location = (int *)(obj + size);

	// Setting the seed for random number generation. This happens once per library load.
	heapsentryu_init();

	// Generating a random number and casting it to size of int.
	int canary = (int)rand();

	// Saving the canary at its designated location.
	*canary_location = canary;

	*(p_group_buffer + group_count * 2) = (size_t) canary_location;
	*(p_group_buffer + group_count * 2 + 1) = (size_t) canary;
	printf("p_group_buffer[%d][0]: %d\n", group_count,
	       (int)*(p_group_buffer + group_count * 2));
	printf("p_group_buffer[%d][1]: %d\n", group_count,
	       (int)*(p_group_buffer + group_count * 2 + 1));
	(group_count)++;

	if (group_count == CANARY_GROUP_SIZE) {
		sys_canary();
		group_count = 0;
	}

	return obj;
}

// If this library is preloaded before any binary execution, then
// this free() will be invoked instead of stdlib free(). After
// performing the actions to be taken in this method, the address
// of real free() is determined and invoked.
//
// Address of real free() is determined using libdl. RTLD_NEXT
// indicates dlsym() to find for the next symbol that goes by the
// provided name.
void free(void *ptr)
{
	real_free rfree = (real_free) dlsym(RTLD_NEXT, "free");
	printf("free from heapsentryu library called.\n");
	return rfree(ptr);
}

size_t sys_canary_init()
{
	size_t r = -1;
	size_t n = (size_t) SYS_CANARY_INIT_NUMBER;
	
	printf
	    ("libheapsentryu:sending: p_group_buffer:[%p] p_group_count:[%p] *p_group_count:[%d]\n",
	     p_group_buffer, &group_count, group_count);
	// Below instruction puts the input parameter in rdx, system call
	// number in rax and triggers an interrupt.
	//
	// The output parameter is stored into the variable 'r' via rax.
	__asm__ __volatile__("int $0x80":"=a"(r):"a"(n),
			     "D"((size_t) p_group_buffer), "S"(n),
			     "d"((size_t) &group_count):"cc", "memory");
	return r;
}

size_t sys_canary()
{

	size_t r = -1;
	size_t n = (size_t) SYS_CALL_NUMBER;
	printf
	    ("libheapsentryu:sending: p_group_buffer:[%p] p_group_count:[%p] group_count:[%d]\n",
	     p_group_buffer, &group_count, group_count);
	// Below instruction puts the input parameter in rdx, system call
	// number in rax and triggers an interrupt.
	//
	// The output parameter is stored into the variable 'r' via rax.
	__asm__ __volatile__("int $0x80":"=a"(r):"a"(n),
			     "D"((size_t) p_group_buffer), "S"(n),
			     "d"((size_t) &group_count):"cc", "memory");
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

	if (!init_done) {
		sys_canary_init();
		srand(time(NULL));
		init_done = 1;
	}
}
