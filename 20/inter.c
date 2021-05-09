#include <stdio.h>
#include <string.h>

#include "inter.h"

void interface_action(void)
{
	printf("\n1 -> Get processes by application name\n");
	printf("2 -> Get processes by user name\n");
	printf("3 -> Get processes by uid\n");
	printf("4 -> Get processes by cpu time\n");
	printf("5 - > quit application\n");
	printf("Select action: ");
}

char* interface_information(int mode)
{
	static char input[50]={0}, command[100]={0};
	char mode_msg[4][40] = {
		"application name <imageName> : ", "user name <userName> : ", "uid <uidNumber> : ", "cpu <minutes>:<seconds>.<hundreths> : "};
	char mode_name[4][8] = {"procs", "user", "uid", "cpu"};

	if(mode == 5) strcpy(command, "quit");
	else {
		printf("%s", mode_msg[mode-1]);
		scanf("%s", input);
		sprintf(command, "%s %s", mode_name[mode-1], input);
	}

	return command;
}
