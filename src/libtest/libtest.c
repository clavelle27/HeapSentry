#include <stdio.h>
#include <stdlib.h>

// This is just a test binary. Here, actual malloc() and free()
// functions are invoked. If libheapsentryu is preloaded, then
// the logs present in libheapsentryu should be printed. This
// way, we could be certain that malloc() and free() are indeed
// hijacked. 
//
// Notice that header file belonging to libheapsentryu is not
// included in this file. This file compiles purely based on
// the symbols present in stdlib. But then, during run-time, the
// binary finds symbols from libheapsentryu, for malloc() and
// free(), before it finds the symbols from stdlib.
int main(int argc, char *argv[], char *envp[]){
	//printf("sizeof(int): %ld\n",sizeof(int));
	//printf("Calling malloc():\n");
	int* a = (int*) malloc(10);
	//printf("malloc returned: %p\n",a);
	*a = 10;
	//printf("value of a(before):%d\n",*a);
	//printf("Calling free():\n");
	free(a);
	//printf("value of a(after):%d\n",*a);
}
