#include <stdio.h>
#define __USE_GNU
#include <dlfcn.h>

typedef void *(*real_malloc) (size_t size);
typedef void (*real_free) (void* ptr);

void *malloc(size_t size)
{
	real_malloc rmalloc = (real_malloc) dlsym(RTLD_NEXT, "malloc");
	printf("malloc from library called.\n");
	void *obj = rmalloc(size);
	return obj;
}

void free(void* ptr)
{
	real_free rfree = (real_free) dlsym(RTLD_NEXT, "free");
	printf("free from heapsentryu library called.\n");
	return rfree(ptr);
}
