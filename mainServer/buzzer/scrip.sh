sudo rmmod buzzer_dev
make clean
make
sudo insmod buzzer_dev.ko
rm app
gcc buzzer_app.c -o app
