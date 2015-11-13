#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
typedef int (*funptr_t) (char *filename);

#define BOOKNAME_LEN 16
#define FILE_LEN     512
FILE *f = NULL;
int open_book(char *file)
{
	printf("Opening Book %s...\n", file);

	/* Load bookdatabase and other stuff */
	if (strcasestr(file, "harry")) {
		f = fopen("HarryPotter.txt", "r");
	}

	else if (strcasestr(file, "kite")) {
		f = fopen("KiteRunner.txt", "r");
	}
	sleep(2);
	if (f != NULL) {
		return 0;
	}

	else {
		return 0;
	}
}

int read_book(char *file)
{
	char buffer[FILE_LEN] = { 0 };
	printf("Reading Book %s...\n", file);
	if (f != NULL) {

		// Read book contents and display to user.
		fseek(f, 0, SEEK_END);
		int length = ftell(f);
		fseek(f, 0, SEEK_SET);
		fread(buffer, 1, length, f);
		fprintf(stdout, "%s\n", buffer);
		sleep(2);
		return 0;
	}

	else {
		return -1;
	}
}

int close_book(char *file)
{
	printf("Closing Book %s...\n", file);
	if (f != NULL) {
		fclose(f);
		sleep(2);
		return 0;
	}

	else {
		return -1;
	}
}

//extern int system(const char *str );

// This is a bookstore. Just enter the name of the book you want to read
int main(int argc, char *argv[])
{
	funptr_t *ptr = NULL;
	char *file = NULL;
	int i = 0;
	printf("BOOKSTORE_PID:%d\n", getpid());
	if (argc < 2) {
		printf("Usage: bookstore <bookname1> <bookname2>\n");
		printf
		    ("       booknames can be \"Harry Potter\" or \"Kite Runner\".\n"
		     "       Spaces in booknames should be escaped by a '\\'\n");
		return -1;
	}
	file = malloc(2 * BOOKNAME_LEN);
	ptr = malloc(3 * sizeof(funptr_t));
	ptr[0] = open_book;
	ptr[1] = read_book;
	ptr[2] = close_book;

	//Copy booknames the user wants to read.
	for (i = 0; i < argc - 1; i++) {
		strcpy(&file[i * BOOKNAME_LEN], argv[i + 1]);
	}

	// Read books one by one.
	for (i = 0; i < argc - 1; i++) {
		int rc = -1;
		rc = ptr[0] (&file[i * BOOKNAME_LEN]);
		if (rc == 0)
			rc = ptr[1] (&file[i * BOOKNAME_LEN]);
		if (rc == 0)
			rc = ptr[2] (&file[i * BOOKNAME_LEN]);
	}
	return 0;
}
