SRCS:=$(shell find . -name \*.c)

.PHONY: bookstore clean

bookstore: $(SRCS:%.c=%.o) 
	$(CC) -o $@ $^

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o bookstore *.swp
