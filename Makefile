all:
	${MAKE} -C src

clean:
	${MAKE} -C src clean

install:
	sudo mkdir -p /etc/heapsentry
	sudo cp ./heapsentry /bin/
