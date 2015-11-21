SRCS:=$(shell find . -name \*.c)

.PHONY: clothstore clean

clothstore: $(SRCS:%.c=%.o) 
	$(CC) -o $@ $^

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o clothstore *.swp
