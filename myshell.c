#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
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

Token *tokenize(const char *string)
{
	Token *t = (Token*)malloc(sizeof(Token));

	t->argc = 0;
	t->isPipe = 0;

	t->argv = (char**)malloc(sizeof(char*) * 50);
	t->argv[0] = (char*)malloc(sizeof(char) * 100);
	char *currentString = t->argv[0];
	int currentStringPosition = 0;

	int i;
	for (i = 0; i <= strlen(string); i++)
	{
		if (string[i] != ' ' && string[i] != '|' && string[i] != '\n' && string[i] != '\0')
		{
			currentString[currentStringPosition++] = string[i];
		}
		else
		{
			if (string[i] == '|')
				t->isPipe = 1;

			if (currentStringPosition > 0)
			{
				currentString[currentStringPosition] = '\0';
				t->argc++;
				currentStringPosition = 0;

				printf("terminated string: '%s'\n", currentString);

				if (string[i] != '\0' && string[i] != '\n')
				{
					t->argv[t->argc] = (char*)malloc(sizeof(char) * 100);
					currentString = t->argv[t->argc];
				}
			}
		}
	}

	t->argv[t->argc] = NULL;

	return t;
}

int (*builtinCommands[])(int argc, char **argv) = {
	[0] cd,
	[1] myexit,
};

int callprogram(int argc, char **argv)
{
	pid_t cpid = fork();

	if(cpid == -1)
	{
		printf("failed to fork\n");
		return 1;
	}
	else if (cpid == 0)
	{
		// child
		printf("gonna execute:\n");

		if (execvp(argv[0], argv) == -1)
		{
			printf("%s\n", strerror(errno));
			// exit(1);
		}
		// error if it reaches this
		return 1;
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
	return -1;	// should never reach this
}

int main(int argc, char **argv)
{
	while (1)
	{
		// printf("%s\n", getenv("PATH"));
		char command[200];
		printf("$ ");
		fgets(command, 200, stdin);

		// start parsing through the command
		if (strncmp(command, "exit", 4) == 0)
			builtinCommands[1](0, NULL);
		// else if (strcmp(command, "cd") == 0)
			// builtinCommands[0]()
		else
		{
			Token *t = tokenize(command);
			int exitval;

			int i = 0;
			for (; t->argv[i] != NULL; i++)
				printf("arg %d: %s\n", i, t->argv[i]);

			if (!t->isPipe)
			 	exitval = callprogram(t->argc, t->argv);

			free(t);

			printf("exited with value %d\n", exitval);
		}
	}
	exit(0);
}
