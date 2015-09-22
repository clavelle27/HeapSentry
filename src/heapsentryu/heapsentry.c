#include <stdio.h>

int heapsentryu_init()
{
    printf("libheapsentryu init function\n");
    printf("libheapsentryu init function - exiting\n");
    return 1;
}

int obj = 10;
void *malloc(size_t size)
{
    printf("malloc from library called.\n");
    return (void *) &obj;
}
