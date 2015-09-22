#include <stdio.h>
#include <stdlib.h>
//#include <heapsentryu.h>

int main(int argc, char *argv[], char *envp[]){
	printf("Calling malloc():\n");
	int* a = (int*) malloc(sizeof(int));
	printf("malloc returned: %p\n",a);
	*a = 10;
	printf("value of a(before):%d\n",*a);
	printf("Calling free():\n");
	free(a);
	printf("value of a(after):%d\n",*a);
}
