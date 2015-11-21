# This is a root makefile. This invokes makefiles of other components.

# These are equivalent to global vars in makefile system.
# `export` directive makes the variables available to all the
# nested makefiles invoked from this makefile. `:=` makes the
# variables, non-modifiable.

CANARY_GROUP_SIZE:=5

# For 64-bit machines
#SYS_CALL_TABLE_ADDRESS=0x$(shell sudo cat /boot/System.map-`uname -a | cut -d " " -f3` | grep ia32_sys_call_table | cut -d " " -f1 )

# For 32-bit machines
SYS_CALL_TABLE_ADDRESS=0x$(shell sudo cat /boot/System.map-`uname -a | cut -d " " -f3` | grep sys_call_table | cut -d " " -f1 )

hs_tune?=100
MEDIUM_RISK_PERCENTAGE:=$(hs_tune)
SYS_CANARY_INIT_NUMBER:=279
SYS_CALL_NUMBER:=280
SYS_CANARY_FREE_NUMBER:=281
ROOT_DIR:=$(shell pwd)
LIB_DIR:=$(ROOT_DIR)/lib
CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Werror -I$(INCLUDE_DIR) -fPIC -g3
INCLUDE_DIR:=$(ROOT_DIR)/include
export ROOT_DIR LIB_DIR INCLUDE_DIR CC CFLAGS SYS_CALL_TABLE_ADDRESS SYS_CALL_NUMBER CANARY_GROUP_SIZE SYS_CANARY_INIT_NUMBER SYS_CANARY_FREE_NUMBER MEDIUM_RISK_PERCENTAGE

all:
	${MAKE} -C src/heapsentryu
	${MAKE} -C src/heapsentryk
	${MAKE} -C src/libtest
	${MAKE} -C test/funptroverwrite
	${MAKE} -C test/clothstore

clean:
	${MAKE} -C src/heapsentryu clean
	${MAKE} -C src/heapsentryk clean
	${MAKE} -C src/libtest clean
	${MAKE} -C test/funptroverwrite clean
	${MAKE} -C test/clothstore clean

install:
	sudo mkdir -p /etc/heapsentry
	sudo cp ./heapsentry /bin/
