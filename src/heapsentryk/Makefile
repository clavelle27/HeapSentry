obj-m := heapsentryk.o
KERNEL_SOURCE := /usr/src/linux
PWD := $(shell pwd)
default:
	KCPPFLAGS="-DSYS_CALL_NUMBER=$(SYS_CALL_NUMBER) -DSYS_CANARY_INIT_NUMBER=$(SYS_CANARY_INIT_NUMBER) -DSYS_CALL_TABLE_ADDRESS=$(SYS_CALL_TABLE_ADDRESS)" ${MAKE} -C /lib/modules/$(shell uname -r)/build M=${PWD} modules
 
clean:
	sudo rm -rf *.o *.order *.symvers *.ko *.mod.c .*.cmd .tmp_versions/
