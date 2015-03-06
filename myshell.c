#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "myshell.h"

int cd(int argc, char **argv)
{
	return 0;
}

int myexit(int argc, char *argv[])
{
	if (argc == 2)
		exit(atoi(argv[1]));
	else
		exit(0);
}

functPtr builtinCommands[] = {
	(functPtr)cd,
	(functPtr)myexit
};

int main(int argc, char **argv)
{
	while (1)
	{
		char command[200];
		printf("$ ");
		scanf("%s", command);

		// start parsing through the command
		if (strcmp(command, "exit") == 0)
			myexit(0, NULL);
	}
	exit(0);
}
