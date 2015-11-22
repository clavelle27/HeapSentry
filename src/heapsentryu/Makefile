LIBS=-ldl
SRCS:=$(shell find . -name \*.c)

.PHONY: libheapsentryu clean

libheapsentryu: $(SRCS:%.c=%.o) 
	@mkdir -p ../../lib
	$(CC) -shared -o ../../lib/$@.so $^ $(LIBS)

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DCANARY_GROUP_SIZE=$(CANARY_GROUP_SIZE) -DSYS_CANARY_NUMBER=$(SYS_CANARY_NUMBER) -DSYS_CANARY_INIT_NUMBER=$(SYS_CANARY_INIT_NUMBER) -DSYS_CANARY_FREE_NUMBER=$(SYS_CANARY_FREE_NUMBER) $(OPTS) -o $@ $^ 

clean:
	rm -rf obj *.o ../../lib/libheapsentryu.so *.swp
