#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#define BUZZER_MAJOR_NUMBER	502
#define BUZZER_MINOR_NUMBER	102
#define BUZZER_DEV_PATH_NAME "/dev/buzzer"

int main(void)
{
	dev_t buzzer_dev;
	int fd;
	int buzzerstate = 0;
	float result;
	
	buzzer_dev = makedev(BUZZER_MAJOR_NUMBER, BUZZER_MINOR_NUMBER);
	mknod(BUZZER_DEV_PATH_NAME, S_IFCHR|0666, buzzer_dev);
	
	fd = open(BUZZER_DEV_PATH_NAME, O_RDWR);
	
	if(fd < 0){
		printf("fail to open buzzer\n");
		return -1;
	}
	
	
	while(buzzerstate<20){
		
		write(fd, &buzzerstate, 4);
		printf("buzzerstate = %d\n",buzzerstate);
		buzzerstate = buzzerstate + 2;
	}
	
	close(fd);
	
	return 0;
}