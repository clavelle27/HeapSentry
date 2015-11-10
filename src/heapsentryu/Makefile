LIBS=-ldl
SRCS:=$(shell find . -name \*.c)

.PHONY: libheapsentryu clean

libheapsentryu: $(SRCS:%.c=%.o) 
	@mkdir -p ../../lib
	$(CC) -shared -o ../../lib/$@.so $^ $(LIBS)

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DCANARY_GROUP_SIZE=$(CANARY_GROUP_SIZE) -DSYS_CALL_NUMBER=$(SYS_CALL_NUMBER) -DSYS_CANARY_INIT_NUMBER=$(SYS_CANARY_INIT_NUMBER) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o ../../lib/libheapsentryu.so *.swp
