CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Werror -I../include -fPIC -g3 

SRCS:=$(shell find . -name \*.c)

.PHONY: testlibu clean

testlibu: $(SRCS:%.c=%.o) 
		$(CC) -o ../$@ $^ 

%.o: %.c
		@mkdir -p $(dir $@)
			$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
		rm -rf obj *.o ../testlibu *.swp
