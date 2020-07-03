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

#define TMP_MAJOR_NUMBER   504
#define TMP_MINOR_NUMBER   104
#define TMP_DEV_PATH_NAME "/dev/tmp"

#define BUFFER_SIZE 1024
void error_handling(char *message) {
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}
int main(int argc, char *argv[])
{
   int sock;
   struct sockaddr_in serv_addr;
   int str_len;
   char s1[20];

   if(argc!=3) {
      printf("Usage : %s <IP> <port>\n", argv[0]);
      exit(1);
   }
   sock=socket(PF_INET, SOCK_STREAM, 0);
   if(sock ==-1)
      error_handling("socket() error");
      
   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
   serv_addr.sin_port=htons(atoi(argv[2]));
   
   if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
      error_handling("connect() error!");
      
   dev_t tmp_dev;
   int fd;
   tmp_dev = makedev(TMP_MAJOR_NUMBER, TMP_MINOR_NUMBER);
   mknod(TMP_DEV_PATH_NAME, S_IFCHR|0666, tmp_dev);
   
   fd = open(TMP_DEV_PATH_NAME, O_RDWR);
   
   if(fd < 0){
      printf("fail to open TMP\n");
      return -1;
   }

   char reg[1] = {0x03}; // temp result register(0x03)
   write(fd, reg, 1);
   float cTemp = 0;
   char data[2] = {0};
   //while(1){
      
      if(read(fd, data, 2) != 2)
      {
         printf("Erorr : Input/output Erorr \n");
      }
      else
      {   
         int temp = (data[0] * 256 + (data[1] & 0xFC)) / 4;
         
         cTemp = temp * 0.03125;
         
         printf("Temperature in Celsius is : %.2f C \n", cTemp);
      }
   //}
   sprintf(s1, "%f", cTemp);
   printf("%s\n", s1);

   write(sock, s1, strlen(s1));
   //sleep(1);
   str_len = read(sock, s1, sizeof(s1)-1);
   if(str_len==-1)
      error_handling("read() error!");
   printf("Message from main server : %s \n", s1);
   
   close(sock);
   close(fd);
   
   return 0;
}
