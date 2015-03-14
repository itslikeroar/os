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
	t->argv[1] = NULL;
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
				stringIsTerminated = 0;
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

int callprogram(Token *t, int in[2], int out[2])
{
	pid_t cpid = fork();

	if (cpid == -1)
	{
		fprintf(stderr, "failed to fork on %s\n", t->argv[0]);
		return 1;
	}
	else if (cpid == 0)
	{
		// child
		if (in != NULL)
		{
			dup2(in[0], 0);
			close(in[1]);
		}

		if (out != NULL)
		{
			dup2(out[1], 1);
			close(out[0]);
		}

		printf("gonna execute: ");
		int i;
		for (i = 0; t->argv[i] != NULL; i++)
			printf("'%s' ", t->argv[i]);
		printf("\n");

		if (execvp(t->argv[0], t->argv) == -1)
		{
			fprintf(stderr, "%s on execvp(%s,...)\n",
				strerror(errno), t->argv[0]);
			// exit(1);
		}
		// error if it reaches this
		// return 1;
		exit(1);
	}
	else
	{
		// parent
		if (in != NULL && out != NULL)
		{
			int exitstatus = 0;
			wait(&exitstatus);
			exitstatus = WEXITSTATUS(exitstatus);
			return exitstatus;
		}
		// wait(0);
	}
	return -1;	// should never reach this
}

int pipecommands(Token **array)
{
	if (array == NULL || array[0] == NULL)
		return -1;

	if (array[1] == NULL)
		return callprogram(array[0], NULL, NULL);
	
	int fd[2];
	pipe(fd);

	pid_t cpid = fork();

	if (cpid == -1)
	{
		fprintf(stderr, "failed to fork on %s\n", array[0]->argv[0]);
		return 1;
	}
	else if (cpid == 0) // child
	{
		dup2(fd[0], 0);
		close(fd[1]);
		int retVal = runcommands(array + 1);
		close(fd[0]);
		return retVal;
		
	}
	else // parent
	{
		int status;
		wait(&status);
		status = WEXITSTATUS(status);

		if (retVal != 0)
			status = retVal;

		return status;
	}
	// should never be here
	return -1;
}

// int runcommands(Token **array)
// {
// 	if (array == NULL || array[0] == NULL)
// 		return -1;

// 	if (array[1] == NULL)
// 		return callprogram(array[0]);

// 	pid_t cpid = fork();

// 	if (cpid == -1)
// 	{
// 		fprintf(stderr, "failed to fork on %s\n", array[0]->argv[0]);
// 		return 1;
// 	}
// 	else if (cpid == 0) // child
// 	{
// 		// makes sure that the original stdout is not redirected
// 		int retVal = pipecommands(array);
// 		exit(retVal);
// 	}
// 	else // parent
// 	{
// 		int status;
// 		wait(&status);
// 		return WEXITSTATUS(status);
// 	}
// }

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
			int exitval;

			int i;
			for (i = 0; tokenArray[i] != NULL; i++)
			{
				printf("token %s:", tokenArray[i]->argv[0]);
				int j;
				for (j = 0; tokenArray[i]->argv[j] != NULL; j++)
					printf("'%s'", tokenArray[i]->argv[j]);
				printf("\n");
			}

			if (tokenArray[1] == NULL)
			{
				printf("only one command\n");
				exitval = callprogram(tokenArray[0], NULL, NULL);

				// MOVE THIS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				printf("exited with value %d\n", exitval);
			}
			else
			{
				printf("such here\n");
				exitval = pipecommands(tokenArray);
				printf("exited with value %d\n", exitval);
			}
			 
			// free(t);

			
		}
	}
	exit(0);
}
