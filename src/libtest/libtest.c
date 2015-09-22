#include <stdio.h>
#include <heapsentryu.h>

int main(int argc, char *argv[], char *envp[]){
	printf("heapsentryu_init returned: %d\n",heapsentryu_init());
	printf("malloc returned: %p\n",(int*)malloc(10));
}
