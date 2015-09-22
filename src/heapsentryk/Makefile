obj-m := heapsentryk.o
KERNEL_SOURCE := /usr/src/linux
PWD := $(shell pwd)
default:
	${MAKE} -C /lib/modules/$(shell uname -r)/build M=${PWD} modules
 
clean:
	sudo rm -rf *.o *.order *.symvers *.ko *.mod.c .*.cmd .tmp_versions/
