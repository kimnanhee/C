#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "inter.h"
#include "logger.h"
extern FILE* fp;

void interface_action(void)
{
	printf("\nPlease select a number from the following menu:\n\n");
	printf("1. Returns processes with the given image.\n");
	printf("2. Returns processes with the given user name.\n");
	printf("3. Returns processes with given uid.\n");
	printf("4. Returns processes that have used > time given.\n");
	printf("5. To quit this session.\n");
	fprintf(fp,"\nPlease select a number from the following menu:\n\n");
	fprintf(fp,"1. Returns processes with the given image.\n");
	fprintf(fp,"2. Returns processes with the given user name.\n");
	fprintf(fp,"3. Returns processes with given uid.\n");
	fprintf(fp,"4. Returns processes that have used > time given.\n");
	fprintf(fp,"5. To quit this session.\n");
}

void interface_information(int mode, char com[])
{
	char input[50]={0};
	char mode_msg[4][100] = {"What image are you looking for: ", "Which user are you looking for: ", "What uid are you looking for: ", "What time would you like to compare with:\n(minutes:seconds.hundreths) "};
	char mode_name[4][10] = {"procs", "user", "uid", "cpu"};

	if(mode == 5)
		sprintf(com, "%s", "quit");
	else 
	{
		printf("%s", mode_msg[mode-1]);
		fprintf(fp,"%s", mode_msg[mode-1]);
		scanf("%s", input);
		sprintf(com, "%s %s", mode_name[mode-1], input);
	}
	return;
}
