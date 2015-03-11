#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

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

int tokenizeArgs(const char *string, char **returnString)
{
	int argcount = 0;

	int i;
	for (i = 0; i < strlen(string); i++)
		printf("%c\n", string[i]);

	return argcount;
}

int (*builtinCommands[])(int argc, char **argv) = {
	[0] cd,
	[1] myexit,
};

int callprogram(int argc, char **argv)
{
	pid_t childpid= fork();

	if(childpid == -1)
	{
		printf("failed to fork\n");
		return 1;
	}
	else if (childpid == 0)
	{
		// child
		// printf("gonna execute %s\n", argv[0]);
		if (execvp(argv[0], argv) == -1)
			printf("%s\n", strerror(errno));
	}
	else
	{
		// printf("here\n");
		// parent
		int exitstatus;
		wait(&exitstatus);
		return exitstatus;
		// wait(0);
	}
	return 0;
}

int main(int argc, char **argv)
{
	while (1)
	{
		// printf("%s\n", getenv("PATH"));
		char command[200];
		printf("$ ");
		scanf("%s", command);

		// start parsing through the command
		if (strcmp(command, "exit") == 0)
			builtinCommands[1](0, NULL);
		// else if (strcmp(command, "cd") == 0)
			// builtinCommands[0]()
		else
		{
			char *string = "gcc";
			char *lies[] = {string, NULL};
			int exitval = callprogram(1, lies);
			printf("exited with value %d\n", exitval);
		}
	}
	exit(0);
}
