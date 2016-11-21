/* servers end created by Chen Xi */  

#include <string.h>  
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>  
#include <netinet/in.h> 
#include <netdb.h>
#include <stdlib.h>  
#include <unistd.h>  
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <arpa/inet.h>

   
#define MAX_BUFFER_SIZE 1024  
#define BACKLOG 10

char command[10];  

  
  /* AWS send UDP packets */
int set_UDP_connection(int integer[], int target_port, int from, int to){
  //int target_port = 21145;
  int socket_udp_fd; 
  int iter = 0;  
  char my_buf[20]; 
  struct hostent *my_host;  
  struct sockaddr_in target_address;
 
  char host_name[MAX_BUFFER_SIZE];
  gethostname(host_name, MAX_BUFFER_SIZE);
      
    /*Get host target_address*/  
  if ((my_host = gethostbyname(host_name)) == NULL)  
  {  
    perror("gethostbyname error"); 

    exit(1);  
  }  

    target_address.sin_family = AF_INET;  
    target_address.sin_addr = *((struct in_addr * )my_host->h_addr);
    target_address.sin_port = htons(target_port);  
  
    socket_udp_fd = socket( AF_INET, SOCK_DGRAM, 0);

    bind(socket_udp_fd,(struct sockaddr *) & target_address, sizeof( target_address));  

    sprintf(my_buf,"%s",command); 
    // send SUM or MAX MIN SOS
    sendto(socket_udp_fd,my_buf, sizeof(my_buf), 0, (struct sockaddr *)& target_address, sizeof( target_address));

    int i;
    for(i = from; i <= to; i++){
      int cur = integer[i];
      sprintf(my_buf,"%d",cur); 
      sendto(socket_udp_fd, my_buf,sizeof(my_buf),0,(struct sockaddr *)& target_address, sizeof( target_address));
    }  
    /* send cancel when finished */
    sprintf(my_buf,"cancel\n");
    
    sendto(socket_udp_fd, my_buf,sizeof(my_buf),0,(struct sockaddr *)& target_address, sizeof( target_address));

    close(socket_udp_fd);  
 
  return 0;
}

/* AWS receives packets throught UDP*/
int receive_udp_packets(int aws_port){

    int length;  
    int integer;
    char buffer_1[20]; 
  
    int socket_fd_1;  
    struct sockaddr_in socket_add;  
     
    socket_add.sin_family = AF_INET;  
    socket_add.sin_addr.s_addr = htonl(INADDR_ANY);  
    socket_add.sin_port = htons(aws_port);  
    length = sizeof(socket_add);  
  
    socket_fd_1 = socket(AF_INET,SOCK_DGRAM,0);  
    bind(socket_fd_1,(struct sockaddr *) & socket_add, sizeof(socket_add));  
    int total = 0;

    int i = 0;
    /* receive messages from server A,B,C*/
    recvfrom(socket_fd_1, buffer_1, sizeof(buffer_1), 0, (struct sockaddr *)&socket_add, &length);  
    
    close(socket_fd_1);  
    int ret;
    ret = atoi(buffer_1);
  return ret;
}

/* Set TCP connectins to client for AWS*/
int TCP_connection(int aws_port, int serverA_port,int serverB_port,int serverC_port ,int aws_tcp_port){
	struct sockaddr_in serv_target_address;
    struct sockaddr_in clnt_target_address;  
    int sin_size;
    int file_size;  
    int socket_fd;
    int client_fd;  
    char buffer[MAX_BUFFER_SIZE];  
    /* set up a socket, set up a TCP connection. */  
    if ((socket_fd = socket(AF_INET,SOCK_STREAM,0))== -1){  
        perror("socket error");  
        exit(1);  
    }  
    
    serv_target_address.sin_family = AF_INET;  
    serv_target_address.sin_port = htons(aws_tcp_port);  
    serv_target_address.sin_addr.s_addr = INADDR_ANY;  

    char y;
    y = '1';
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof y);      
      
    if (bind(socket_fd, (struct sockaddr *) &serv_target_address, sizeof(struct sockaddr))== -1){  
        perror("bind error");

        exit(1);  
    }  

    if (listen(socket_fd, BACKLOG) < 0){  
        perror("listen error");  
        exit(1);  
    }  

    // while   
    
    while(1){
      printf("AWS: The AWS is up and running.\n");  
      sin_size = sizeof( struct sockaddr); // new changes 
    if ((client_fd = accept(socket_fd, (struct sockaddr *)&clnt_target_address, &sin_size)) < 0){  
        perror("accept error");  
        exit(1);  
    }  
    memset(buffer , 0, sizeof(buffer));



    if ((file_size = recv(client_fd, buffer, MAX_BUFFER_SIZE, 0)) < 0){  
        perror("Receive error number 2.");  
        exit(1);  
    }  
    sprintf(command,"%s",buffer); 

    if(send(client_fd,"received message.", sizeof "received message.",0) < 0){
           perror("send error!");  
           close(client_fd);  
           exit(0);  
    }  

    int integer[1024];
    //memset(file , 0, MAX_BUFFER_SIZE);
    int reveive_integer;
    reveive_integer = recv(client_fd, integer, (sizeof integer)+1, 0);
    if(reveive_integer < 0){
      perror("Receive error number 1.");  
      close(client_fd);  
      exit(0);  
    }
    

    int total_len = integer[0];

    printf("AWS: The AWS has received %i numbers from the client using TCP over port %i\n", total_len, aws_tcp_port );
    /* divide number into 3 groups */
    int len1;
    len1 = (total_len + 1)/3;

    int len2;
    len2 = 2 * ((total_len + 1)/3);

    int len3;
    len3 = total_len;

    int numberA, numberB, numberC;

    printf("AWS: The AWS sent %i numbers to Backend­Server A \n", len1);

    /* Send to server A*/
    set_UDP_connection(integer, serverA_port,1 , len1);
    numberA = receive_udp_packets(aws_port);
    

    /* Send to server B*/
    printf("AWS: The AWS sent %i numbers to Backend­Server B \n", len2 - len1);
    set_UDP_connection(integer, serverB_port,len1 + 1, len2);
    numberB = receive_udp_packets(aws_port);
    

    /* Send to server C*/
    printf("AWS: The AWS sent %i numbers to Backend­Server C \n", len3 - len2);
    set_UDP_connection(integer, serverC_port, len2 + 1, len3);
    numberC = receive_udp_packets(aws_port);
    printf("The AWS received reduction result of %s from Backend­Server A using UDP over port %i and it is %i\n", command, aws_port, numberA);
    printf("The AWS received reduction result of %s from Backend­Server B using UDP over port %i and it is %i\n", command, aws_port, numberB);
    printf("The AWS received reduction result of %s from Backend­Server C using UDP over port %i and it is %i\n", command, aws_port, numberC);

    /* do the calculate with the command */
    int my_ret = 0;
    if(strncmp(command, "SUM", 3) == 0 || strncmp(command, "sum", 3) == 0){
        my_ret = numberA + numberB + numberC;
    }else if(strncmp(command, "SOS", 3) == 0 || strncmp(command, "sos", 3) == 0){
        my_ret = numberA + numberB + numberC;
    }else if(strncmp(command, "MAX", 3) == 0 || strncmp(command, "max", 3) == 0){
        my_ret = numberA;
        if(my_ret < numberB) my_ret = numberB;
        if(my_ret < numberC) my_ret = numberC;

    }else if(strncmp(command, "MIN", 3) == 0 || strncmp(command, "min", 3) == 0){
        my_ret = numberA;
        if(my_ret > numberB) my_ret = numberB;
        if(my_ret > numberC) my_ret = numberC;
    }

    char retstr[30];
    sprintf(retstr, "%i",my_ret);

    printf("AWS: The AWS has successfully finished the reduction %s: %i\n", command, my_ret); 
    
    if(send(client_fd,retstr ,sizeof retstr,0) < 0){
           perror("send error!");  
           close(client_fd);  
           exit(0);  
    }  


    
    printf("AWS: The AWS has successfully finished sending the reduction value to client.\n \n");

    close(client_fd);
  }  
  return 0;
}

/* set up a UDP connection and receive data dor server A B C*/
int setup_udp_reveive(int this_port, int aws_port, char myname){
    int length;  
    int integer;
    char buffer_1[20]; 
    char buffer_2[20];
    char name[5];
    sprintf(name, "%c", myname);
    //name = myname; 
    printf("Server %s: The Server %s is up and running using UDP on port %i. \n",name , name , this_port); 
  
    int socket_fd_1;  
    /* assign attributes to the socket*/
    struct sockaddr_in socket_add;   
     
    socket_add.sin_family = AF_INET;  
    socket_add.sin_addr.s_addr = htonl(INADDR_ANY);  
    socket_add.sin_port = htons(this_port);  
    length = sizeof(socket_add);  
  
    socket_fd_1 = socket(AF_INET,SOCK_DGRAM,0);  
    bind(socket_fd_1,(struct sockaddr *) & socket_add, sizeof(socket_add));  
    int total = 0;
    int key = 1;
    /*receive SOS MIN MAX*/
     recvfrom(socket_fd_1, buffer_1, sizeof(buffer_1), 0, (struct sockaddr *)&socket_add, &length);  
    sprintf(command, "%s",buffer_1);
    int number_reveived;
    number_reveived = 0;

    /* continue receive UDP packets using while */
    while(1)  
    {  
        recvfrom(socket_fd_1, buffer_2, sizeof(buffer_2), 0, (struct sockaddr *)&socket_add, &length); 
        int cur = atoi(buffer_2); 
        /* if reveive a string cancel, means the mission completed*/
        if(strncmp( buffer_2, "cancel", 5) == 0) {  
            printf("Server %s: The Server %s has received %i numbers. \n",name, name, number_reveived);  
            set_UDP_send(aws_port, total, command, name);  
            break;  
        } 
        if(strncmp(buffer_1, "SUM", 3) == 0 || strncmp(command, "sum", 3) == 0){
            total += cur;
        }
        else if(strncmp(buffer_1, "MAX", 3) == 0 || strncmp(command, "max", 3) == 0){
            if(total < cur) total = cur;
        }
        else if(strncmp(buffer_1, "MIN", 3) == 0 || strncmp(command, "min", 3) == 0){
            if(key == 1) total = cur;
            if(total > cur) total = cur;
            key = 2;
        }
        else if(strncmp(buffer_1, "SOS", 3) == 0 || strncmp(command, "sos", 3) == 0){
            total += cur*cur;
        }else{
            perror("Wrong input.");
            break;
        }
        number_reveived++;
        
    }  
    close(socket_fd_1);  
    
    return 0;  

 }

/* Server send packets result back to AWS  */
 int set_UDP_send(int target_port,int number, char string[], char name[]){

  int socket_udp_fd; 
  struct hostent *my_host;  
  struct sockaddr_in target_address;
  int iter=0;  
  char my_buffer[20]; 
  char host_name[MAX_BUFFER_SIZE];
  gethostname(host_name, MAX_BUFFER_SIZE);
    /*Get host target_address*/  
  if ((my_host = gethostbyname(host_name)) == NULL)  
  {  
    perror("gethostbyname error");  
    exit(1);  
  }  

    target_address.sin_family = AF_INET;  
    target_address.sin_addr = *((struct in_addr * ) my_host->h_addr);
    target_address.sin_port = htons(target_port);  
    /* Create a socket here*/
    socket_udp_fd=socket(AF_INET,SOCK_DGRAM,0);  
  
    sprintf(my_buffer,"%i",number);

    sendto(socket_udp_fd,my_buffer,sizeof(my_buffer),0,(struct sockaddr *)& target_address, sizeof( target_address));
    close(socket_udp_fd); 
    printf("Server %s: The Server %s has successfully finished the reduction %s: %i \n", name, name, command, number);
    printf("Server %s: The Server %s has successfully finished sending the reduction value to AWS server.\n \n",name, name);  
     
  return 0;
}


int main(int argc, char *argv[])  
{  

    // int aws_port = 24145;
    // int serverA_port = 21145;
    // int serverB_port = 22145;
    // int serverC_port = 23145;
    // int aws_tcp_port = 25145;

    int aws_port = atoi(argv[1]);
    int serverA_port = atoi(argv[2]);
    int serverB_port = atoi(argv[3]);
    int serverC_port = atoi(argv[4]);
    int aws_tcp_port = atoi(argv[5]);

    

    pid_t pid = fork();
    if(pid == 0){

        pid_t pid_clild_1 = fork();
        if(pid_clild_1 == 0){
            while(1){
                //printf("The Server A is up and running using UDP on port %i. \n", serverA_port);  
                char name;
                name = 'A';
                setup_udp_reveive(serverA_port, aws_port, name);
            }
        }else{
            while(1){
                //printf("The Server B is up and running using UDP on port %i. \n", serverB_port);  
                char name;
                name = 'B';
                setup_udp_reveive(serverB_port, aws_port, name);
            }
        }
        
    }else{
    	pid_t pid_clild_2 = fork();
    	if(pid_clild_2 == 0){
    		while(1){
            //printf("The Server C is up and running using UDP on port %i. \n", serverC_port);  
            	char name;
            	name = 'C';
            	setup_udp_reveive(serverC_port, aws_port, name);
        	}
    	}else{
        	TCP_connection(aws_port, serverA_port, serverB_port, serverC_port , aws_tcp_port);

    	}


    }

    exit(0);  
} 
