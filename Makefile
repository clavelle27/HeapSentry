all:
	${MAKE} -C src/heapsentryu
	${MAKE} -C src

clean:
	${MAKE} -C src/heapsentryu clean
	${MAKE} -C src clean

install:
	sudo mkdir -p /etc/heapsentry
	sudo cp ./heapsentry /bin/
