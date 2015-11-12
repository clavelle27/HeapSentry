#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	printf("Entering libtest\n");
	//printf("sizeof(int): %ld\n",sizeof(int));
	//printf("Calling malloc():\n");
	int* a = (int*) malloc(10);
	int* b = (int*) malloc(10);
	int* c = (int*) malloc(10);
	int* d = (int*) malloc(10);
	int* e = (int*) malloc(10);
	int* f = (int*) malloc(10);
	*a = 10;
	*b = 10;
	*c = 10;
	*d = 10;
	*e = 10;
	*f = 10;
	free(a);
	free(b);
	free(c);
	free(d);
	free(e);
	free(f);
	printf("Exiting libtest\n");
}
