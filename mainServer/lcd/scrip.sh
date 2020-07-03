cd ../buzzer
sudo rmmod buzzer_dev
make clean
make
sudo insmod buzzer_dev.ko

cd ../servo
sudo rmmod servo_dev
make clean
make
sudo insmod servo_dev.ko

cd ../lcd
sudo rmmod lcd_dev
make clean
make
sudo insmod lcd_dev.ko
rm app
gcc lcd_app.c -o app
