#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>

void error(char *msg)
{
	perror(msg);
	exit(0);
}

void sig_handler(int sig) 
{
    /* Handle signals */
    if (sig == SIGILL || sig == SIGSEGV) 
        printf("Can not process data\n");

    if (sig == SIGPIPE)
        printf("Broken Pipe\n");

    if (sig == SIGINT) 
        printf("Cleanup on exit\n");

    exit(-1);
}

char *get_basic_info()
{
	struct utsname buffer;
	char *data = malloc(1024); //i think its enough size
   	int errno = 0;
   	if (uname(&buffer) == 0)
    	{
	   	sprintf(data,"system name = %s\n", buffer.sysname);
   		sprintf(data,"%s; node name   = %s\n",data, buffer.nodename);
   		sprintf(data,"%s; release     = %s\n", data,buffer.release);
   		sprintf(data,"%s; version     = %s\n",data, buffer.version);
   		sprintf(data,"%s; machine     = %s\n",data, buffer.machine);

	   #ifdef _GNU_SOURCE
	      sprintf(data,"domain name = %s\n", buffer.domainname);
	   #endif
   }
   else
   {
   	strcpy(data, "no result");
   }
   
   return data;
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno,clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	int n,an;
	
	/* Read signals */
    signal(SIGPIPE, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGILL , sig_handler);
    
	if(argc<2)
	{
		fprintf(stderr, "Error no port provided\n");
		exit(0);
	}
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr,sizeof(serv_addr));

	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
	{
		error("ERROR binding socket");
	}
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	while (1) 
	{
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
		if(newsockfd<0)
		{
			error("ERROR on accept");
		}
		
		while ((n = recv(newsockfd,buffer,255,0)) > 0)
		{
			buffer[1023] = '\0';
			printf("Here is the message %s\n", buffer);

			if(strstr(buffer, "x") != NULL)
			{
				close(sockfd);
				close(newsockfd);
				exit(1);
			}

			/* Get and send anwer */
			char *answer = get_basic_info();
			send(newsockfd, answer, strlen(answer), 0);
			free(answer);
		}
	}
}
