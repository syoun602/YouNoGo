#include <unistd.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <sys/ioctl.h> 
#include <sys/types.h> 
#include <sys/sysmacros.h>  
#include <sys/stat.h>

#define ULT_MAJOR_NUMBER	505
#define ULT_MINOR_NUMBER	105
#define SERVO_MAJOR_NUMBER  501
#define SERVO_MINOR_NUMBER  101
#define ULT_DEV_PATH_NAME 	 "/dev/ultrasonic_dev"
#define SERVO_DEV_PATH_NAME "/dev/servo_dev"

#define INTERVAL 		50000


int main (int argc, char ** argv ){ 

	dev_t ultrasonic_dev, servo_dev;
	int fd1, fd2;
	int result, val;
	char push = 'p';
	
	printf("Sanitizer Auto Spray\n"); 
	
	ultrasonic_dev = makedev(ULT_MAJOR_NUMBER, ULT_MINOR_NUMBER);
	mknod(ULT_DEV_PATH_NAME, S_IFCHR|0666, ultrasonic_dev);
	
	servo_dev = makedev(SERVO_MAJOR_NUMBER, SERVO_MINOR_NUMBER);
	mknod(SERVO_DEV_PATH_NAME, S_IFCHR|0666, servo_dev);
	
	
	fd1 = open(ULT_DEV_PATH_NAME, O_RDWR);
	fd2 = open(SERVO_DEV_PATH_NAME, O_RDWR);
	
	if(fd1 < 0){
		printf("fail to open ultrasonic\n");
		return -1;
	}
	if(fd2 < 0){
		printf("fail to upen servo\n");
		return -1;
	}
	printf("----- Detecting.... -----\n");
	while(1){
		read(fd1, &val, sizeof(int));
		result = val/58.0;
		printf("distance = %d cm\n", result);
		
		if(result < 20){
			printf("Spraying Sanitizer\n");
			write(fd2, &push, sizeof(char));
			sleep(1);
			printf("----- Detecting.... -----\n");
		}
		sleep(1);
	}
	
	close(fd1); 
	close(fd2); 
} 
