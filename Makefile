all:
	${MAKE} -C src/heapsentryu
	${MAKE} -C src/libtest

clean:
	${MAKE} -C src/heapsentryu clean
	${MAKE} -C src/libtest clean

install:
	sudo mkdir -p /etc/heapsentry
	sudo cp ./heapsentry /bin/
