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

Cmds *CommandsCreate()
{
	Cmds *commands = (Cmds*)malloc(sizeof(Cmds));
	commands->tokens = (Token**)malloc(sizeof(Token*) * 50);
	commands->count = 0;
	return commands;
}

Cmds *tokenize(const char *string)
{
	Cmds *commands = CommandsCreate();
	// Token **tokenArray = (Token**)malloc(sizeof(Token*) * 50);
	Token **tokenArray = commands->tokens;
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

	commands->count = tokenNum + 1;

	return commands;
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
			// printf("[%d]%s input from %ld\n", getpid(), t->argv[0], in);
			close(in[1]);
			dup2(in[0], 0);
			close(in[0]);
		}

		if (out != NULL)
		{
			// printf("[%d]%s outputing to %ld\n", getpid(), t->argv[0], out);
			close(out[0]);
			dup2(out[1], 1);
			close(out[1]);
		}

		// printf("[%d]gonna execute: ", getpid());
		// int i;
		// for (i = 0; t->argv[i] != NULL; i++)
		// 	printf("'%s' ", t->argv[i]);
		// printf("\n");

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
		if (in == NULL && out == NULL)
		{
			printf("executed without any redirects\n");
			int exitstatus;
			wait(&exitstatus);
			exitstatus = WEXITSTATUS(exitstatus);
			printf("exited with value %d\n", exitstatus);
			return exitstatus;
		}
		else
			return 0; // don't wait yet
		// wait(0);
	}
	return -1;	// should never reach this
}

void pipecommands(Cmds *commands)
{
	pid_t cpid;
	int status, i;
	int fdArray[50][2];

	for (i = 0; i < commands->count - 1; i++)
	{
		printf("making pipe %d\n", i);
		pipe(fdArray[i]);
	}

	for (i = 0; i < commands->count; i++)
	{
		int *pipeIn = NULL;
		int *pipeOut = NULL;

		if (i != 0)
			pipeIn = fdArray[i - 1];

		if (i != commands->count - 1)
			pipeOut = fdArray[i];

		printf("callprogram %s with pipeIn: %ld (pipeArray[%d]), pipeOut: %ld (pipeArray[%d])\n",
			commands->tokens[i]->argv[0], pipeIn, i - 1, pipeOut, i);
		callprogram(commands->tokens[i], pipeIn, pipeOut);

		if (i - 1 >= 0)
		{
			printf("closing pipe %d\n", i);
			close(fdArray[i-1][0]);
			close(fdArray[i-1][1]);
		}
	}

	// for (i = 0; i < commands->count - 1; i++)
	// {
	// 	close(fdArray[i][0]);
	// 	close(fdArray[i][1]);
	// }

	while ((cpid = wait(&status)) != -1)
		printf("child %d exits with %d\n", cpid, WEXITSTATUS(status));
}

int main(int argc, char **argv)
{
	char command[200];
	while (printf("$ "), fgets(command, 200, stdin))
	{
		// start parsing through the command
		if (strncmp(command, "exit", 4) == 0)
			builtinCommands[1](0, NULL);
		// else if (strcmp(command, "cd") == 0)
			// builtinCommands[0]()
		else
		{
			Cmds *commands = tokenize(command);
			printf("commands->count: %d\n", commands->count);
			Token **tokenArray = commands->tokens;
			// int exitval;

			int i;
			for (i = 0; i < commands->count; i++)
			{
				printf("token %s:", tokenArray[i]->argv[0]);
				int j;
				for (j = 0; tokenArray[i]->argv[j] != NULL; j++)
					printf("'%s'", tokenArray[i]->argv[j]);
				printf("\n");
			}

			if (commands->count == 1)
			{
				printf("only one command\n");
				callprogram(tokenArray[0], NULL, NULL);
			}
			else
			{
				printf("such here\n");
				pipecommands(commands);
				// printf("pipe commands exited with value %d\n", exitval);
			}
			 
			// free(t);

			
		}
	}
	printf("\n");
	exit(0);
}
