SRCS:=$(shell find . -name \*.c)

.PHONY: libtest clean

libtest: $(SRCS:%.c=%.o) 
	$(CC) -o ../../$@ $^

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o ../../libtest *.swp
