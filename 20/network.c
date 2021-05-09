#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "network.h"

void error_handling(char *msg)
{
        fputs(msg, stderr);
        fputc('\n', stderr);
        exit(0);
}

int socket_init(void)
{
	struct sockaddr_in serv_addr;
	int sock;

        sock = socket(PF_INET, SOCK_STREAM, 0); // make socket
        if(sock == -1) error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serv_addr.sin_port = htons(atoi("7060"));

        if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error");

	return sock;
}
