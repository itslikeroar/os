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

int tokenize(char *commands[50][50], char string[])
{
	int commandNum = 0;
	int argNum = 0;
	int lastArgStart = 0;
	// int argLen = 0;
	int quoted = 0;	// 0 = not quoted, 1 = single, 2 = double

	// saving this since adding null characters
	// will cause this to change
	int length = strlen(string);

	commands[0][0] = NULL;	// initializing argv[0]
	// *commands[0] = NULL;		// initializing commandsArray[0]

	int i;
	for (i = 0; i < length; i++)
	{
		if (quoted == 0)
		{
			if (string[i] == '\'')
				quoted = 1;
			else if (string[i] == '"')
				quoted = 2;
			else if (string[i] == '|')
			{
				if (*commands[commandNum] == NULL)
				{
					perror("piping without command\n");
					return -1;
				}

				string[i] = '\0';
				commands[commandNum++][++argNum] = NULL;
				argNum = 0;
			}
			else if (string[i] == '\n' || string[i] == ' ')
				string[i] = '\0';
			else
			{
				if (commands[commandNum][argNum] == NULL)
				{
					printf("commands[%d][%d] = %s\n", commandNum, argNum, string + i);
					commands[commandNum][argNum] = string + i;
					lastArgStart = i;
					commands[commandNum][argNum + 1] = NULL;
				}
				else if (strlen(commands[commandNum][argNum]) < i - lastArgStart)
				{
					commands[commandNum][argNum] = string + i;
					lastArgStart = i;
					commands[commandNum][argNum + 1] = NULL;;
				}
			}
		}
		else if (quoted == 1 && string[i] == '\'')
		{
			string[i] = '\0';
			argNum++;
			quoted = 0;
		}
		else if (quoted == 2 && string[i] == '"')
		{
			string[i] = '\0';
			argNum++;
			quoted = 0;
		}
	}

	if (quoted != 0)
	{
		perror("missmatched quotes\n");
		return -1;
	}

	commands[commandNum][++argNum] = NULL;
	// *commands[++commandNum] = NULL;

	return commandNum + 1;
}

int (*builtinCommands[])(int argc, char **argv) = {
	[0] cd,
	[1] myexit,
};

// int callprogram(Token *t, int in[2], int out[2])
// {
// 	pid_t cpid = fork();

// 	if (cpid == -1)
// 	{
// 		fprintf(stderr, "failed to fork on %s\n", t->argv[0]);
// 		return 1;
// 	}
// 	else if (cpid == 0)
// 	{
// 		// child
// 		if (in != NULL)
// 		{
// 			// printf("[%d]%s input from %ld\n", getpid(), t->argv[0], in);
// 			close(in[1]);
// 			dup2(in[0], 0);
// 			close(in[0]);
// 		}

// 		if (out != NULL)
// 		{
// 			// printf("[%d]%s outputing to %ld\n", getpid(), t->argv[0], out);
// 			close(out[0]);
// 			dup2(out[1], 1);
// 			close(out[1]);
// 		}

// 		// printf("[%d]gonna execute: ", getpid());
// 		// int i;
// 		// for (i = 0; t->argv[i] != NULL; i++)
// 		// 	printf("'%s' ", t->argv[i]);
// 		// printf("\n");

// 		if (execvp(t->argv[0], t->argv) == -1)
// 		{
// 			fprintf(stderr, "%s on execvp(%s,...)\n",
// 				strerror(errno), t->argv[0]);
// 			// exit(1);
// 		}
// 		// error if it reaches this
// 		// return 1;
// 		exit(1);
// 	}
// 	else
// 	{
// 		// parent
// 		if (in == NULL && out == NULL)
// 		{
// 			printf("executed without any redirects\n");
// 			int exitstatus;
// 			wait(&exitstatus);
// 			exitstatus = WEXITSTATUS(exitstatus);
// 			printf("exited with value %d\n", exitstatus);
// 			return exitstatus;
// 		}
// 		else
// 			return 0; // don't wait yet
// 		// wait(0);
// 	}
// 	return -1;	// should never reach this
// }

// void pipecommands(Cmds *commands)
// {
// 	pid_t cpid;
// 	int status, i;
// 	int fdArray[50][2];

// 	for (i = 0; i < commands->count - 1; i++)
// 	{
// 		printf("making pipe %d\n", i);
// 		pipe(fdArray[i]);
// 	}

// 	for (i = 0; i < commands->count; i++)
// 	{
// 		int *pipeIn = NULL;
// 		int *pipeOut = NULL;

// 		if (i != 0)
// 			pipeIn = fdArray[i - 1];

// 		if (i != commands->count - 1)
// 			pipeOut = fdArray[i];

// 		printf("callprogram %s with pipeIn: %ld (pipeArray[%d]), pipeOut: %ld (pipeArray[%d])\n",
// 			commands->tokens[i]->argv[0], pipeIn, i - 1, pipeOut, i);
// 		callprogram(commands->tokens[i], pipeIn, pipeOut);

// 		if (i - 1 >= 0)
// 		{
// 			printf("closing pipe %d\n", i);
// 			close(fdArray[i-1][0]);
// 			close(fdArray[i-1][1]);
// 		}
// 	}

// 	// for (i = 0; i < commands->count - 1; i++)
// 	// {
// 	// 	close(fdArray[i][0]);
// 	// 	close(fdArray[i][1]);
// 	// }

// 	while ((cpid = wait(&status)) != -1)
// 		printf("child %d exits with %d\n", cpid, WEXITSTATUS(status));
// }

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
			char *commandsArray[50][50];
			// memset(commandsArray, 0, sizeof(char) * 50 * 50 * 50);

			int numCommands = tokenize(commandsArray, command);
			printf("number of commands: %d\n", numCommands);

			int i;
			for (i = 0; i < numCommands; i++)
			{
				printf("args for %s: ", commandsArray[i][0]);
				int j;
				for (j = 0; commandsArray[i][j] != NULL; j++)
					printf("'%s' ", commandsArray[i][j]);
				printf("\n");
			}

			// if (commands->count == 1)
			// {
			// 	printf("only one command\n");
			// 	callprogram(commandsArray[0], NULL, NULL);
			// }
			// else
			// {
			// 	printf("such here\n");
			// 	pipecommands(commandsArray);
			// 	// printf("pipe commands exited with value %d\n", exitval);
			// }
			 
			// free(t);

			
		}
	}
	printf("\n");
	exit(0);
}
