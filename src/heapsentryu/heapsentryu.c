#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define __USE_GNU
#include <dlfcn.h>

// These are the function pointers used to hold the address of 
// actual system calls. The syntax of these function pointers
// should precisely match with the syntax of real system calls.
typedef void *(*real_malloc) (size_t size);
typedef void (*real_free) (void *ptr);
size_t sys_canary(int a1);

// If this library is preloaded before any binary execution, then
// this malloc() will be invoked instead of stdlib malloc(). After
// performing the actions to be taken in this method, the address
// of real malloc() is determined and invoked.
//
//  Address of real malloc() is determined using libdl. RTLD_NEXT
//  indicates dlsym() to find for the next symbol that goes by the
//  provided name.
void *malloc(size_t size)
{
	printf("malloc from libheapsentryu called.\n");

	// Determine the function pointer of libc malloc().
	real_malloc rmalloc = (real_malloc) dlsym(RTLD_NEXT, "malloc");

	// sizeof(int) depends on the compiler and not the hardware.
	// So, it is not good to assume it to be 4 or 8 or whatever.
	size_t modified_size = size + sizeof(int);

	// Requesting malloc to allocate space for an extra integer along with `size` bytes.
	char *obj = (char*)rmalloc(modified_size);
	
	// Determining the canary location to store the random number and casting it to int*.
	int *canary_location = (int*) (obj + size);
	printf("canary_location: %p\n",canary_location);

	// Setting the seed for random number generation. If the seed is known,
	// then the random numbers can be reproduced in the same sequence, since
	// this is a pseudo-random number generator.
	// 
	// Hence, setting a new seed everytime, so it cannot be reproduced again,
	// and the number generated is not reproducible and fresh.
	srand(time(NULL));

	// Generating a random number and casting it to size of int.
	int canary = (int) rand();

	*canary_location = canary;
	printf("canary: %d\n",*canary_location);

	// TODO: Fill up an array (probably) of size configured by user and pass it
	// to kernel when it gets filled.
	sys_canary(10);
	
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

size_t sys_canary(int a1){
	size_t r = -1;
	// TODO: Set the value of N from a macro value passed from makefile.
	// Use similar approach to set the system call number in kernel.
	size_t n = 280;
	__asm__ __volatile__("int $0x80":"=a"(r):"a"(n), "D"((size_t)a1):"cc", "memory");
	printf("SYS_sentry returned: %ld\n",r);
	return r;
}





