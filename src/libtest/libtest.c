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
	int *a[21];
	int i = 0;
	for (i = 0; i< 18;i++){
		a[i] = malloc(10);
	}
	free(a[16]);
	a[18] = malloc(10);
	a[19] = malloc(10);
	a[20] = malloc(10);

	for (i=0;i<21;i++){
		if (i != 16)
	free(a[i]);
	}
	printf("Exiting libtest\n");
}
