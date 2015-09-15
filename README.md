Instructions:
`make` compile
`make install` for installation

Project Description:
Course project for CSE509:System Security. (Fall 2015)
Heap overflows are as deadly as stack overflows. An attacker can take advantage
of a heap overflow to overwrite a function pointer and eventually execute
malicious code. Despite their importance, heap objects are much less protected
than a program's stack.

In this project, you are called to implement HeapSentry, a system that your
advisor described and published in DIMVA 2013. HeapSentry consists of a user-space
component and a kernel-level component. The user-space component intercepts all
memory allocation calls and adds a "canary" at the end of each block. It also
communicates these new canaries (and their locations) to the kernel, which will
check the liveness of these canaries right before a dangerous system call is
performed.

Read the paper http://securitee.org/files/heapsentry_dimva2013.pdf in detail to
find out more of the specifics and feel free to use different technologiesfor
hooking into the appropriate kernel-level functions.

Team Members:
(Stony Brook University - Fall 2015)
Ashish Chaudhary
Pavan Manjunath
Shashi Ranjan
Tabish Ahmad
Vishal Nayak
