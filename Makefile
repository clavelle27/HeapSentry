ROOT_DIR:=$(shell pwd)
LIB_DIR:=$(ROOT_DIR)/lib
INCLUDE_DIR:=$(ROOT_DIR)/include
export ROOT_DIR LIB_DIR INCLUDE_DIR
all:
	${MAKE} -C src/heapsentryu
	${MAKE} -C src/libtest

clean:
	${MAKE} -C src/heapsentryu clean
	${MAKE} -C src/libtest clean

install:
	sudo mkdir -p /etc/heapsentry
	sudo cp ./heapsentry /bin/
