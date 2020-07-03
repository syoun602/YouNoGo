sudo rmmod tmp
make clean
make
sudo insmod tmp.ko
rm app
gcc -o app app.c
sudo ./app
