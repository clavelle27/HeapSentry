CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Werror -I../../include -fPIC -g3 
LIBS=-L../../lib -lheapsentryu
SRCS:=$(shell find . -name \*.c)

.PHONY: libtest clean

libtest: $(SRCS:%.c=%.o) 
		$(CC) -o ../../$@ $^ $(LIBS)

%.o: %.c
		@mkdir -p $(dir $@)
			$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
		rm -rf obj *.o ../../libtest *.swp
