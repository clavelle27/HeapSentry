#Disable ASLR
export LD_PRELOAD=./lib/libheapsentryu.so
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space

if [ "$1" != "normal" ]; then
	./test/funptroverwrite/bookstore /bin/bash `perl -e 'printf "A" x 24 . "\x90\x61\xe5\xb7"'`
else
	./test/funptroverwrite/bookstore HarryPorter.txt KiteRunner.txt
fi

#Renable ASLR
echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
