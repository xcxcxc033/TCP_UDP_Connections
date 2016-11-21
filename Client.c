/*Create by Chen Xi*/  

#include <unistd.h>  
#include <netdb.h>  
#include <netinet/in.h>  
#include <stdlib.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>   
#include <string.h>  
#include <sys/ioctl.h> 
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <arpa/inet.h>


  
#define MAX_BUFFER_SIZE 1024  
  
int main(int argc, char *argv[])  
{  
  if(argc != 2){  
    fprintf(stderr,"Please input ./client sum/max/min/sos\n");  
    exit(1);  
  }  
  if(strncmp(argv[1], "max", 3) != 0 && strncmp(argv[1], "min", 3) != 0 && strncmp(argv[1], "sos", 3) != 0 && strncmp(argv[1], "sum", 3) != 0 && strncmp(argv[1], "max", 3) != 0 ){
    fprintf(stderr,"Please input ./client sum/max/min/sos\n");  
    exit(1);  
  }

  if(strncmp(argv[1], "max", 3) == 0){
    strncpy(argv[1],"MAX",3);
  }else if(strncmp(argv[1], "min", 3) == 0){
    strncpy(argv[1],"MIN",3);
  }else if(strncmp(argv[1], "sos", 3) == 0){
    strncpy(argv[1],"SOS",3);
  }else if(strncmp(argv[1], "sum", 3) == 0){
    strncpy(argv[1],"SUM",3);
  }
    


    int socket_fd;
    int file_size;  
    char buffer[MAX_BUFFER_SIZE];  
    struct hostent *my_host;  
    struct sockaddr_in serv_addr;  
    int aws_port = 25145;

    printf("The client is up and running.\n");
      
    
    char temp[MAX_BUFFER_SIZE];
    gethostname(temp, MAX_BUFFER_SIZE);
      
    /*Get host address*/  
    if ((my_host = gethostbyname(temp)) == NULL)  
    {  
        perror("get host name error");  
        exit(1);  
    }  
      
    memset(buffer, 0, sizeof(buffer));  
    
    //sprintf(buffer, "%s", argv[1]);  
      
    /*create a socket */  
    if ((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)  
    {  
        perror("socket create error");  
        exit(1);  
    }  
      
    /*assign attributes of the struct sockaddr_in*/  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_port = htons(aws_port);  
    serv_addr.sin_addr = *((struct in_addr * )my_host->h_addr);  

      
    /* connect the client to the server*/  
    if(0 > connect(socket_fd,(struct sockaddr * ) &serv_addr, sizeof(struct sockaddr))){  
        perror("connect error");  
        exit(1);  
    }  
    /* Store comment into buffer */  
    strcpy(buffer,argv[1]);

    printf("The client has sent the reduction type %s to AWS.\n", buffer);

    /* Send first message to server SUM/ MIN/ MAX/ SOS  */  
    if ((file_size = send(socket_fd, buffer, sizeof(buffer), 0)) == -1){  
        perror("send error");  
        exit(1);  
    }  

    int file_size2;

    file_size2 = recv( socket_fd, buffer, sizeof( buffer), 0);


     /* read csv file, and store into interger array */ 
    FILE * nums_file = fopen("nums.csv", "r");
    if(NULL == nums_file){
      printf("%s\n", "No such file, please check.");
      exit(1);  
    } 

    char buffer_2[20];
    int i = 1;
    char * store[1024];

    char *line;
    int integer[1024];
    memset(integer,0,1024);
    /* use wile to read each line, and store into interger array */ 
    while(NULL != (line = fgets(buffer_2, sizeof(buffer_2) , nums_file))){
        integer[i] = atoi(buffer_2);
        i++;
    }
    integer[0] = i - 1;
    fclose(nums_file);

    if (0 > send(socket_fd, integer, (sizeof integer) + 1, 0))  
    {  
        perror("send error 2");  
        exit(1);  
    }

    printf("The client has sent %i numbers to AWS.\n", i-1);


    char final_sult[20];
    if(0 > (file_size2=recv(socket_fd,final_sult, sizeof(final_sult),0))){  
        perror("connect error 3");  
        exit(1);  
    } 

    printf("The client has received reduction %s: %s\n", argv[1], final_sult);

    close(socket_fd);  
    exit(0);  
}  











