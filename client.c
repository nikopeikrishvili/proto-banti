#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
int tcp_creat_soc (const char *host, const char *port);
int tcp_creat_data_soc (const char *port);
void clean();

int commandsocfd, datasocfd, rcvfd;

/*
  This function creates a socket and uses it to connect to the server.
  parameters: host and port.
  return value: file descriptor for outgoing socket that is connected to 'host'
 */
int tcp_creat_soc (const char *host, const char *port) {
    int rv, sockfd;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof((struct addrinfo)hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, port, &hints, &res)) != 0) {
        fprintf(stderr, "Client: getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("Client: Socket");
    }
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        close(sockfd);
        perror("client:connect");
        return -1;
    }
    printf("client: connecting to %s \n", host);
    freeaddrinfo(res);
    return sockfd;
}
/*
  This function creates a listening socket used by the client to listen for incoming data connections from the server
  parameter: port
  return value: returns a file descriptor for a listening socket that is waiting for a data connection from the server.
*/
int tcp_creat_data_soc(const char* port) {
    int status, datasockfd;
    struct addrinfo hints, *res;
    memset(&hints, 0 , sizeof((struct addrinfo)hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "data getaddrinfo: %s\n", gai_strerror(status));
    }

    if ((datasockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("Client Data Socket");
    }

    if (bind(datasockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Bind");
    }

    if (listen(datasockfd, 1) < 0) {
        perror("data listen");
    }
    //printf("Listening for data connection on data socket %d \n", sockfd);
    return datasockfd;
}

void clean() {
    close(commandsocfd);
    close(datasocfd);
    close(rcvfd);
    exit(0);
}

/*
  This main function does the following in chronological order.

  1. Calls tcp_creat_soc, which connects toe the server
  2. Calls gethostname to get the host name of the client machine and stores it in msg.
  3. Uses send() to send msg to the server.
  4. Waits for confirmation from the server using the recv() fuction.
  5. Creates and binds a data socket using port 30020 by calling tcp_creat_data_soc.
  6. Parses user input using fgets() on stdin.
  7. Sends command using send().
  8. Rcvs confirmation message from server.
  9. Does appropiate action based of confirmation message.

 */

int main (int argc, char *argv[]) {
    socklen_t addr_size;
    struct sockaddr_storage their_addr;
    int len, bytes_sent, bytes_rcv, status;
    const char* host = argv[1];
    const char* port = argv[2];
    char rbuf[1000], msg[1000], *cmd, *token;
    size_t size = sizeof(msg);
    FILE* file;
    len = sizeof(rbuf);

    memset(rbuf, 0, sizeof(rbuf));

    signal(SIGINT, clean);
    signal(SIGTERM, clean);

    if (argc != 3) {
        fprintf(stderr, "usage: client hostname port\n");
        exit(1);
    }

    commandsocfd = tcp_creat_soc(argv[1], argv[2]);
    if (commandsocfd < 1) {
        printf("Connection failed. \n");
        exit(0);
    }
    if (gethostname(msg, size) < 0) {
        perror("gethostname");
    }
    bytes_sent = send(commandsocfd, msg, len, 0);
    bytes_rcv = recv(commandsocfd, msg, len, 0);


    datasocfd = tcp_creat_data_soc("30020");
    if ((bytes_rcv = recv(commandsocfd, rbuf, len, 0)) < 0) {
        perror("recv");
        exit(0);
    }
    printf("%s", rbuf);


    while (1) {
        fgets(msg, len, stdin);
        if (send(commandsocfd, msg, len, 0) < 0) {
            perror("send");
            exit(0);
        }

        if ((bytes_rcv = recv(commandsocfd, rbuf, len, 0)) < 0) {
            perror("recv");
            exit(0);
        }
        printf("%s\n", rbuf);

        if (strcmp(rbuf, "resend") == 0) {
            printf("Invalid Command\n");
            continue;
        } else if (strcmp(rbuf, "exit") == 0) {
            printf("Bye\n");
            clean();
            exit(0);
        } else if (strcmp(rbuf, "write") == 0) {
            cmd = strdup(msg);
            token = strsep(&cmd, " ");
            token = strsep(&cmd, "\n");
        } else if (strcmp(rbuf, "fnf") == 0) {
            printf("File not found: %s\n", token);
            continue;
        }


        rcvfd = accept(datasocfd, (struct sockaddr *)&their_addr, &addr_size);
        if (strcmp(rbuf, "write") == 0) {
            memset(rbuf, 0, sizeof(rbuf));
            if (access(token, F_OK) == 0) {
                /*
                  If the file exists, accept the incoming data and do nothing with it.
                */
                printf("File exists already\n");
                while ((bytes_rcv = recv(rcvfd, rbuf, len, 0)) != 0) {
                }
            } else {
                if ((file = fopen(token, "wb")) == NULL ) {
                    perror("fopen");
                }
                while ((bytes_rcv = recv(rcvfd, rbuf, len, 0)) != 0) {
                    printf("bytes_rcv %d \n", bytes_rcv);
                    fwrite(rbuf, 1, bytes_rcv, file);
                }
                fclose(file);
            }
        } else {
            while ((bytes_rcv = recv(rcvfd, rbuf, len, 0)) != 0) {
                printf("%s", rbuf);
            }
        }
    }


    clean();
    return 0;
}