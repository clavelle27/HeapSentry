## Instructions:
Run `$sudo cat /boot/System.map-3.13.0-61-generic | grep sys_call_table`
Use value of `ia32_sys_call_table` for `SYS_CALL_TABLE_ADDRESS` in root `Makefile`
Compile using `$ make`
Insert the kernel module using `sudo insmod src/heapsentryk/heapsentryk.ko`
Test the library using `./run.sh`
See kernel logs using `tail -f /var/log/syslog` or `dmesg`
Note: Remove the kernel module using `sudo rmmod heapsentryk`


## Project Description:
Course project for CSE509:System Security. (Fall 2015)
Heap overflows are as deadly as stack overflows. An attacker can take advantage
of a heap overflow to overwrite a function pointer and eventually execute
malicious code. Despite their importance, heap objects are much less protected
than a program's stack.

In this project, HeapSentry as described in HeapSentry_Paper.pdf (included).
HeapSentry consists of a user-space component and a kernel-level component.
The user-space component intercepts all memory allocation calls and adds a
"canary" at the end of each block. It also communicates these new canaries
(and their locations) to the kernel, which will check the liveness of these
canaries right before a dangerous system call is performed.

Read HeapSentry_Paper.pdf in detail to find out more of the specifics.

## Stony Brook University - Project Team:

Ashish Chaudhary, Pavan Manjunath, Shashi Ranjan, Tabish Ahmad, Vishal Nayak


ATTACK CODE:

1. Testing 'control-data' (function pointer overwrite) attack using 'Bookstore' example.
The exploit code is a contrived version of a bookstore where user can specify books that he wants
to read on the command line. The bookstore then opens and displays the contents of the book to
the user. There are only two books in the store - HarryPorter.txt ( you can specify anything that has
"harry" inside it to read this book) and KiteRunner.txt( you can specify anything that has "kite" inside
it to read this book). One way of reading both books is-
./bookstore Harry Kite
Or
./bookstore HarryPotter.txt KiteRunner.txt

This opens and displays one book at a time with some delay in b/w just to mimic book loading etc

Now if you wanto run the exploit code, we will do it like this-
./bookstore /bin/bash `perl -e 'printf "A" x 32 . "\xf0\xa8\xa5\xf7\xff\x7f\x00\x00"'`

The fancy hex numbers in the perl printf is nothing but the address of system() function
in libc on my system, which is 0x00007ffff7a5a8f0. On your machine it would be different
and you need to find it out once by hooking on to a simple hello world program in gdb.
The exploit code overwrites a function pointer that's original purpose was to open the user desired book with
the address of system() function. And instead of passing the book's name to the open_book func, we now
pass /bin/bash to system() with this exploit.

GDB disables ASLR by default for the debugged process. Hence the above address of system()
is the same across multiple runs of the program. But without GDB, we have to do it ourselves.
Sample code is shown in run.sh in the same dir for reference. run.sh can run both normal as well as exploit code.
Just pass "normal" as command line param to run.sh to run the bookstore in normal fashion. With no params, exploit 
code will run.

2. Testing 'non-control data' (local variable) using 'Clothstore' example.
clothstore sells different clothes of different prices. The goal of this attack is to overwrite the price of a product with a lower price so that we can purchase costly products for lesser price. As we are not hijacking the control flow, this is a Non_Control Attack. We are modifying a non-control variable for our gains.

In order to launch an attack, just get the malloced location of the product user wants t buy and the malloced location of the existing products in the store. Just enter as many As as the difference between these two locations plus 1. For example if prod is on heap at 0x1000 and prod_list is at 0x120, write 33 A's as the product you want to purchase. This will cut the cost of the first product from $500 to $256. Note that since we are entering all As that doesnt match with any product in the store, you will not able to purchase anything in the first attempt. But on a second attempt, you will be able to purchase the first product for the rigged price.

