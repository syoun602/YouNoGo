#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>


#define SERVO_MAJOR_NUMBER  501
#define SERVO_MINOR_NUMBER  101
#define SERVO_DEV_PATH_NAME "/dev/servo"

#define BUZZER_MAJOR_NUMBER	502
#define BUZZER_MINOR_NUMBER	102
#define BUZZER_DEV_PATH_NAME "/dev/buzzer"

#define LCD_MAJOR_NUMBER	503
#define LCD_MINOR_NUMBER	103
#define LCD_DEV_PATH_NAME "/dev/lcd"


#define BUFFER_SIZE 1024
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
int main(int argc, char *argv[])
{
	dev_t lcd_dev, buzzer_dev, servo_dev;
	int fd_buzzer, fd_lcd, fd_servo;
	char pass[10];
	int buzzerstate = 0;
	char push = 'p';

	lcd_dev = makedev(LCD_MAJOR_NUMBER, LCD_MINOR_NUMBER);
	mknod(LCD_DEV_PATH_NAME, S_IFCHR|0666, lcd_dev);
	fd_lcd = open(LCD_DEV_PATH_NAME, O_RDWR);
	if(fd_lcd < 0){
		printf("fail to open LCD\n");
		return -1;
	}

	buzzer_dev = makedev(BUZZER_MAJOR_NUMBER, BUZZER_MINOR_NUMBER);
	mknod(BUZZER_DEV_PATH_NAME, S_IFCHR|0666, buzzer_dev);
	fd_buzzer = open(BUZZER_DEV_PATH_NAME, O_RDWR);
	if(fd_buzzer < 0){
		printf("fail to open buzzer\n");
		return -1;
	}	

	servo_dev = makedev(SERVO_MAJOR_NUMBER, SERVO_MINOR_NUMBER);
	mknod(SERVO_DEV_PATH_NAME, S_IFCHR|0666, servo_dev);
	fd_servo = open(SERVO_DEV_PATH_NAME, O_RDWR);
	if(fd_servo < 0){
		printf("fail to open buzzer\n");
		return -1;
	}

	int serv_sock;
	int clnt_sock;
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	
	char temp[BUFFER_SIZE];

	
	if(argc!=2){
		printf("length error\n");
		exit(1);
	}
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
	if(clnt_sock ==-1)
		error_handling("accept() error");

	char msg[] = "trasnfer complete!!";
	write(clnt_sock, msg, sizeof(msg));
	read(clnt_sock, temp, sizeof(temp));
	printf("temporature is %s\n", temp);

	close(clnt_sock);
	close(serv_sock);
	
	int tempo=atoi(temp);

	if(tempo >=40){
		printf("temp is 40C up\n");
		strcpy(pass, "0"); // 
	}
	else if(tempo<40){
		printf("temp is 40C down\n");
		strcpy(pass, "1");
	}
	strcat(pass, temp);
	write(fd_lcd, pass, 1024);
	close(fd_lcd);

	// 40C up
	if(tempo >=40){ 
		while(buzzerstate<20){
			write(fd_buzzer, &buzzerstate, 4);
			buzzerstate = buzzerstate + 2;
		}
		close(fd_buzzer);
	}
	// 40C Down(Door open).
	else if(tempo<40){ 
		write(fd_servo, &push, sizeof(char));
		sleep(1);
		sleep(1);
		close(fd_servo);
	}
	
	return 0;
}

