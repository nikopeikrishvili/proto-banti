
#include "include.h"
#include "commander.h"

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

int call_function(const char *name)
{

  int i;
  for (i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++) {
    if (strcmp(function_map[i].name, name)==0  && function_map[i].func) {
      	function_map[i].func();

      return 0;
    }
    else
    {

    }
  }

  return -1;
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	
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
		
		bzero(buffer, 1024);
		while ((n = recv(newsockfd,buffer,255,0)) > 0)
		{
			if(strstr(buffer, "x") != NULL)
			{
				close(sockfd);
				close(newsockfd);
				exit(1);
			}
		}

		/* Null terminate buffer */
		buffer[1023] = '\0';

		/* Call function */
		call_function(buffer);
		printf("%s\n", buffer);
	}
}
