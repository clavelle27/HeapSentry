# Project Description:
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