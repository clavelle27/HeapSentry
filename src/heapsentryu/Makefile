LIBS=-ldl
SRCS:=$(shell find . -name \*.c)

.PHONY: libheapsentryu clean

libheapsentryu: $(SRCS:%.c=%.o) 
	$(CC) -shared -o ../../lib/$@.so $^ $(LIBS)

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DSYS_CALL_NUMBER=$(SYS_CALL_NUMBER) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o ../../lib/libheapsentryu.so *.swp
