CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Werror -I$(INCLUDE_DIR) -fPIC -g3 
#LIBS=-L$(LIB_DIR) -lheapsentryu
SRCS:=$(shell find . -name \*.c)

.PHONY: libtest clean

libtest: $(SRCS:%.c=%.o) 
	$(CC) -o ../../$@ $^

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o ../../libtest *.swp
