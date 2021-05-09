#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "inter.h"
#include "network.h"

void* readThread(void* args);
int sock;

int block = 0;

int main(int argc, char *argv[])
{
	sock = socket_init();
	
	pthread_t tid;
	pthread_create(&tid, NULL, readThread, NULL); // make thread
	
	int mode = 0;
	char command[100];

	while(1)
	{
		while(block);
		interface_action();
		scanf("%d", &mode); // input action
	
		if(1 <= mode && mode <= 5) {
			memset(command, 0, sizeof(command));
			strcpy(command, interface_information(mode));
			write(sock, command, strlen(command));
		}
		else {
			printf("wrong input action\n");
			continue;
		}
		block = 1;
	}
	close(sock);
	return 0;
}

void* readThread(void* args)
{
	while(1)
	{
		char buff[1024]={0};
		read(sock, buff, sizeof(buff));
		printf("%s", buff);
		
		if(strcmp(buff, "Bye!\n") == 0) {
		       close(sock);
		       exit(0);
		}	       
		block = 0;
	}
}
