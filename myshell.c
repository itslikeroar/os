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

Token *TokenCreate()
{
	Token *t = (Token*)malloc(sizeof(Token));
	t->argv = (char**)malloc(sizeof(char*) * 50);
	t->argv[0] = (char*)malloc(sizeof(char) * 100);
	return t;
}

Token **tokenize(const char *string)
{
	Token **tokenArray = (Token**)malloc(sizeof(Token*) * 50);
	tokenArray[0] = TokenCreate();
	Token *currentToken = tokenArray[0];

	// isQuoted = 0;

	currentToken->argc = 0;
	int tokenNum = 0;

	char *currentString = currentToken->argv[0];
	int currentStringPosition = 0;

	int stringIsTerminated = 0;

	int i;
	for (i = 0; i <= strlen(string); i++)
	{
		if (string[i] != ' ' && string[i] != '|' && string[i] != '\n' && string[i] != '\0')
		{
			if (stringIsTerminated)
			{
				currentToken->argv[currentToken->argc] = (char*)malloc(sizeof(char) * 100);
				currentString = currentToken->argv[currentToken->argc];
			}

			currentString[currentStringPosition++] = string[i];
		}
		else
		{
			if (currentStringPosition > 0)
			{
				currentString[currentStringPosition] = '\0';
				currentToken->argc++;
				currentStringPosition = 0;

				// printf("terminated string: '%s'\n", currentString);

				if (string[i] != '\0' && string[i] != '\n')
				{
					stringIsTerminated = 1;
					// currentToken->argv[currentToken->argc] = (char*)malloc(sizeof(char) * 100);
					// currentString = currentToken->argv[currentToken->argc];
				}
			}

			if (string[i] == '|')
			{
				currentToken->argv[currentToken->argc] = NULL;
				tokenArray[++tokenNum] = TokenCreate();
				currentToken = tokenArray[tokenNum];
			}
		}
	}

	currentToken->argv[currentToken->argc] = NULL;
	tokenArray[tokenNum + 1] = NULL;

	return tokenArray;
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
		// printf("gonna execute:\n");

		if (execvp(argv[0], argv) == -1)
		{
			printf("%s\n", strerror(errno));
			// exit(1);
		}
		// error if it reaches this
		// return 1;
		exit(1);
	}
	else
	{
		// printf("here\n");
		// parent
		int exitstatus = 0;
		waitpid(cpid, &exitstatus, WUNTRACED | WUNTRACED);
		exitstatus = WIFEXITED(exitstatus);
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
			Token **tokenArray = tokenize(command);
			// int exitval;

			if (tokenArray[1] == NULL)
			{
				// printf("only one command\n");
				exitval = callprogram(t->argc, t->argv);

				// MOVE THIS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				printf("exited with value %d\n", exitval);
			}
			else
			{
				printf("many pipe\n");
			}
			 
			// free(t);

			
		}
	}
	exit(0);
}
