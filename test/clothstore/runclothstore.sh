#Disable ASLR
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
if [ "$1" == "protect" ]; then
	export LD_PRELOAD=../../lib/libheapsentryu.so
fi
./clothstore 

#Renable ASLR
echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
