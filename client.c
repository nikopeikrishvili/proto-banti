/*-----------------------------------------------------
Name: client.c
Date: 2015.05.12
Desc: Listener and Commander
-----------------------------------------------------*/


#include "include.h"


/*---------------------------------------------
| Cleanup on exit
---------------------------------------------*/
void cleanup(int sig)
{
    printf("\nCleanup on exit\n");
    exit(0);
}


/*---------------------------------------------
| Create socket
---------------------------------------------*/
int create_socket(uint16_t port)
{
    /* Used variables */
    int sock;
    struct sockaddr_in name;

    /* Create the socket. */
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Give the socket a name. */
    name.sin_family = AF_INET;
    name.sin_port = htons (port);
    name.sin_addr.s_addr = htonl (INADDR_ANY);

    /* Connect socket */
    if (connect(sock, (struct sockaddr *) &name, sizeof(name)) < 0)
    {
        perror("Connect");
        exit(EXIT_FAILURE);
    }

    return sock;
}


/*---------------------------------------------
| Recieve response from server
---------------------------------------------*/
int get_response(int sock) 
{
    /* Used variables */
    static struct sockaddr_in peer;
    socklen_t len;
    char buf[512];
    int newsock, ret  = 0;

    /* Accept socket */
    len = sizeof(struct sockaddr);
    newsock = accept(sock, (struct sockaddr *) &peer, &len);

    /* Read from socket */
    bzero(buf, 512);
    ret = read(newsock, buf, sizeof(buf));

    /* Print response */
    if (ret > 0) 
    {
        printf("%s\n", buf);
        ret = 1;
    }
    else ret = 0;

    /* Cleanup */
    close(newsock);
    return ret;
}


/*---------------------------------------------
| Send command to server via socket
---------------------------------------------*/
int send_command(int sock, int type, char *key) 
{
    /* Used variables */
    char command[256];
    int ret = 0;

    /* Create command */
    bzero(command, 256);
    strcpy(command, key);

    /* Send */
    ret = send(sock, command, strlen(command), 0);
    if (ret < 0) return 0;
    else printf("Sent command: %s\n", command);

    ret = get_response(sock);

    return ret;
}


/*---------------------------------------------
| Main function
---------------------------------------------*/
int main(int argc, char *argv[])
{
    /* Used variables */
    int socket, ret, type = 0;

    /* Check arguments */
    if (!argv[2]) 
    {
        printf("Usage: %s <port> <command>\n\n", argv[0]);
        exit(0);
    }

    /* Catch exit signal*/
    signal(SIGINT, cleanup);

    /* Create socket */
    socket = create_socket(atoi(argv[1]));

    /* Send commant to server */
    ret = send_command(socket, type, argv[2]);

    /* Check and print status */
    if (!ret) printf("Without response\n");
    else printf("Success\n");

    /* Cleanup */
    close(socket);
    return 0;
}
