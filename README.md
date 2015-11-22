CSE 509 System Security
Course Project - HeapSentry

## 1. Team:
Ashish Chaudhary: SBU ID - 109770154
Pavan Manjunath: SBU ID - 109916057
Shashi Ranjan: SBU ID - 109974495
Tabish Ahmad Nisar Ahmad: SBU ID - 109915386
Vishal Nayak: SBU ID - 109892702

## 2. Project Description:
This project implements the idea outlined by the paper HeapSentry: “Kernel-assisted Protection against
Heap Overflows”. Heap overflow attacks are thwarted using a preloaded library that interacts with its
counterpart, which is a kernel module. These components will interact with each other, identify the exploit
and take corrective action to ensure security. This is achieved by creating a fresh canary for every invocation
of malloc/calloc/realloc and associating it with the allocated memory location. The canary value will be
placed at the end of the allocated block. Heap overflow is likely to overwrite the value present at the canary
location. This information will be communicated to the kernel, which checks for correctness of these values
before any of the high-risk and medium-risk system calls are processed.

## 3. Setup and Usage
### 3.1 Platform
This program is developed for 32 bit Linux Operating System. The program is tested on Ubuntu 14.04 32
bit machine.

### 3.2 Useful commands
Following are the commands that are useful to test the program. Compile and insert the module before
running the provided attack code.

Useful commands to be invoked from HeapSentry directory:

- Compiling the source
$ make

- Compiling the source along with performance tuning arguments
$ make CANARY_GROUP_SIZE=10 MEDIUM_RISK_PERCENTAGE=30

- Purging the build artifacts
make clean

- Inserting kernel module
sudo insmod src/heapsentryk/heapsentryk.ko

- Removing the kernel module
sudo rmmod heapsentryk

### 3.3 Instructions
- Load HeapSentryK module
- Preload HeapSentryU library
- Run the desired binary

Please refer Project Report for further details.
