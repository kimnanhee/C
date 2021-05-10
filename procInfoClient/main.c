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
#include "logger.h"
extern FILE* fp;

void* readThread(void* args);
int sock;

int block = 0;

int main(int argc, char *argv[])
{
	sock = socket_init();
	log_begin("_log.txt");
	pthread_t tid;
	pthread_create(&tid, NULL, readThread, NULL); // make thread
	
	int mode = 0;
	
	printf("Welcome to the Info Process Client!\n");
	fprintf(fp,"Welcome to the Info Process Client!\n");
	while(1)
	{
		while(block);
		interface_action();
		scanf("%d", &mode); // input action
	
		if(1 <= mode && mode <= 5) {
			char command[100];
			memset(command, 0, strlen(command));
			interface_information(mode, command);
			printf("Sent: %s\n", command);
			fprintf(fp, "Sent: %s\n", command);
			write(sock, command, strlen(command));
		}
		else {
			printf("wrong input action\n");
			fprintf(fp,"wrong input action\n");
			continue;
		}
		block = 1;
	}
	close(sock);
	if (fp)
		fclose(fp);
	return 0;
}

void* readThread(void* args)
{
	while(1)
	{
		char buff[1024]={0};
		read(sock, buff, sizeof(buff));
		printf("%s", buff);
		fprintf(fp,"%s", buff);
		if(strcmp(buff, "Bye!\n") == 0) 
		{
		       close(sock);
		       printf("\nThanks for using Proc Info Server!\n");
			   fprintf(fp,"\nThanks for using Proc Info Server!\n");
		       exit(0);
		}	       
		block = 0;
	}
}
