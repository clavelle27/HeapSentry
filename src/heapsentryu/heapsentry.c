#include <stdio.h>
#define __USE_GNU
#include <dlfcn.h>

// These are the function pointers used to hold the address of 
// actual system calls. The syntax of these function pointers
// should precisely match with the syntax of real system calls.
typedef void *(*real_malloc) (size_t size);
typedef void (*real_free) (void* ptr);

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
	real_malloc rmalloc = (real_malloc) dlsym(RTLD_NEXT, "malloc");
	printf("malloc from library called.\n");
	void *obj = rmalloc(size);
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
void free(void* ptr)
{
	real_free rfree = (real_free) dlsym(RTLD_NEXT, "free");
	printf("free from heapsentryu library called.\n");
	return rfree(ptr);
}
