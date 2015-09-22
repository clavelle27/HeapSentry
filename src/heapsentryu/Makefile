CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Werror -I$(INCLUDE_DIR) -fPIC -g3 
LIBS=-ldl
SRCS:=$(shell find . -name \*.c)

.PHONY: libheapsentryu clean

libheapsentryu: $(SRCS:%.c=%.o) 
	$(CC) -shared -o ../../lib/$@.so $^ $(LIBS)

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o ../../lib/libheapsentryu.so *.swp
