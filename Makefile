# This is a root makefile. This invokes makefiles of other components.

# These are equivalent to global vars in makefile system.
# `export` directive makes the variables available to all the
# nested makefiles invoked from this makefile. `:=` makes the
# variables, non-modifiable.

SYS_CALL_TABLE_ADDRESS:=0xffffffff81809ca0
SYS_CALL_NUMBER:=280
ROOT_DIR:=$(shell pwd)
LIB_DIR:=$(ROOT_DIR)/lib
CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Werror -I$(INCLUDE_DIR) -fPIC -g3
INCLUDE_DIR:=$(ROOT_DIR)/include
export ROOT_DIR LIB_DIR INCLUDE_DIR CC CFLAGS SYS_CALL_TABLE_ADDRESS SYS_CALL_NUMBER

all:
	${MAKE} -C src/heapsentryu
	${MAKE} -C src/heapsentryk
	${MAKE} -C src/libtest

clean:
	${MAKE} -C src/heapsentryu clean
	${MAKE} -C src/heapsentryk clean
	${MAKE} -C src/libtest clean

install:
	sudo mkdir -p /etc/heapsentry
	sudo cp ./heapsentry /bin/
