CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Werror -I../../include -fPIC -g3 

SRCS:=$(shell find . -name \*.c)

.PHONY: heapsentryu clean

heapsentryu: $(SRCS:%.c=%.o) 
		$(CC) -shared -o ../../$@.so $^ 

%.o: %.c
		@mkdir -p $(dir $@)
			$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
		rm -rf obj *.o ../../heapsentryu.so *.swp
