cd ../ultrasonic
sudo rmmod ultrasonic_dev
make clean
make
sudo insmod ultrasonic_dev.ko
cd ../servo
sudo rmmod servo_dev
make clean
make
sudo insmod servo_dev.ko
cd ../control_app
gcc -o app app.c
