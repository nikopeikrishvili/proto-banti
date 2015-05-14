#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
int tcp_creat_soc (const char *port); //
int tcp_data_connect (const char *host, const char *port);//
int send_file (const char *host, char *filename);//
int list(const char *host);  
void clean(); 
int send_command(int controlfd, char* msg, size_t size);
int listenersocfd, controlfd, datafd; 
/*
  This function creates a listening socket used to accept incoming connections from clients
  parameters: port 
  return value: file descriptor for a socket listening for client connections  
 */

int tcp_creat_soc (const char *port){
  int status, sockfd;
  int yes = 1;
  struct addrinfo hints, *p, *res;
  
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if((status = getaddrinfo(NULL, port, &hints, &res))!= 0){
    
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    clean();
    exit(0);
  }

   
  for(p = res; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
             p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
           sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }


  if(listen(sockfd,10) < 0){
    perror("SERVER: LISTEN");
    clean();
    exit(0);
  }
  printf("Listening on socket %d \n",sockfd);
  return sockfd;
}
/*
  This function creates a socket and uses it to establish a data connection with the client. 
  parameters: host and port
  return value: file descriptor for outgoing socket connected to client. 
 */


int tcp_data_connect (const char *host, const char *port){
  int rv;
  struct addrinfo hints;
  struct addrinfo *res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  printf(host);
  if ((rv = getaddrinfo(host, port, &hints, &res)) != 0){
    clean();
    perror("getaddrinfo");
    
  }
  printf("creating socket");
  datafd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (datafd == -1){
    clean();
    perror("server: Socket");
  }  
  printf("server: attempting connection to %s \n", host);
  
  if(connect(datafd, res->ai_addr, res->ai_addrlen) == -1) {
    close(datafd);
    clean();
    perror("server: connect");

  }
  printf("server: connecting to %s \n", host);
  freeaddrinfo(res);
  return datafd;
}

int list(const char* host) {
  int sendfd, status;
  char buf[1000]; 
  FILE* file;
  sendfd = tcp_data_connect(host, "30020");
  file = popen("ls", "r");
  if(file == NULL) {
    close(sendfd);
    pclose(file);
    perror("popen");
    exit(0);
  }
    
  send_command(sendfd, "\n", sizeof(buf)); 
  while(1){
    status = fgets(buf, 1000, file);
    if(feof(file)) break;
    send_command(sendfd, buf, sizeof(buf));
  }
  send_command(sendfd, "\n", sizeof(buf));
  pclose(file);
  close(sendfd);
  return status;
}

int send_file(const char *host, char *filename) {
  FILE* send_file;
  char buf[100];
  int datasocfd, status; 
  if(filename == NULL){
    return -1;
  }  
  
  status = access(filename, F_OK);
  printf("%s",filename);
  if((send_file = fopen(filename, "r"))==NULL) {
    perror("fopen");
    return -1;
  }
   
  datasocfd = tcp_data_connect(host, "30020");
  status = 1;
  int count = 0;
  do{
    count = fread(buf, 1, sizeof(buf), send_file);
    send_command(datasocfd, buf, count); 
  } while(count == sizeof(buf) );
  //printf("%s sent \n", filename);
  close(datasocfd);
  fclose(send_file);
  return 1;
}
 

void clean () {
  close(listenersocfd);
  close(controlfd);
  close(datafd);
  exit(0);
} 

int send_command (int controlfd, char* msg, size_t size) {
  int status;
  status = send(controlfd, msg, size, 0);
  if(status < 0) {
    clean();
    perror("send");
    exit(0);
  }
  return status;
}



int main (int argc, char *argv[]) {
  socklen_t addr_size;
  struct sockaddr_storage their_addr;
  int status, bytes_rcv, bytes_sent, len;
  const char* port = "30021"; 
  char commandbuf[1000], *their_host, *str;
  
  len = 1000;
  listenersocfd = tcp_creat_soc(port);

  signal(SIGINT, clean);
  signal(SIGTERM, clean);
  signal(SIGSEGV, clean);
  while(1) 
  {
    controlfd = accept(listenersocfd, (struct sockaddr *)&their_addr, &addr_size);
  
    if((bytes_rcv = recv(controlfd,commandbuf, len, 0)) < 0) 
    {
      clean();
      perror("recv");
      exit(0);
    }
    printf("Receiving connection from '%s'.\n", commandbuf);
  
    their_host = strdup(commandbuf);
  
    sprintf(commandbuf, "Succesfully connected to server.\n");    
    send_command(controlfd, commandbuf, sizeof(commandbuf));
  
    status = 1;
    sprintf(commandbuf, "Please enter a command.  Type 'help' to see a list of \n command.  Type 'quit' to exit.\n");
    send_command(controlfd, commandbuf, sizeof(commandbuf));
  
  
    while(status !=0){
      status = recv(controlfd, commandbuf, len, 0);  
      
      if(status < 0) 
      {
            clean();
            perror("rcv");
            exit(0);
      }
       else if(status == 0) 
        {
            printf("connection with %s has ended \n", their_host);
            break;
        } 
        else if((strncmp(commandbuf,"list", 4))== 0) 
        {
            send_command(controlfd, "ok", sizeof("ok"));
            list(their_host);
        } 
        else if((strncmp(commandbuf,"quit", 4))== 0) 
        {
            send_command(controlfd,"exit", sizeof("exit"));
            close(controlfd);
            printf("connection with %s has ended. \n", their_host);
            break;
        }
        else if(strncmp(commandbuf, "get", 3)==0) 
        {  
            //send_command(controlfd, "write");
            str = strdup(commandbuf);
            char* filename = strsep(&str, " ");
            filename = strsep(&str, "\n");
        if(send_file(their_host, filename) < 0)
        {
            printf("File not found\n");
            send_command(controlfd, "fnf", sizeof("fbf")); 
        }
        else
            {
                send_command(controlfd, "write", sizeof("write"));} 
            } 
            else 
            {
                send_command(controlfd, "resend", sizeof("resend"));
            }     
      
        }
  }
  close(listenersocfd);
  close(controlfd);
  close(listenersocfd);

  return 0;
}
