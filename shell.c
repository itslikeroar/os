/*
*
* Operating Systems: CS416 Assignment 5
* 
* Group Members:
* Rodrigo Pacheco
* Brian Yoo
* Sisheng Zhang
* 
*/
#include "shell.h"
#define homeDirectory getenv("HOME")
char path[100];

int cd(int argc, char **argv)
{
    char *dir = NULL;
    if (argc == 1)
        dir = homeDirectory;
    else if (argc == 2)
        dir = argv[1];
    else
    {
        fprintf(stdout, "Error: Too many arguments.\n");
        return 1;
    }

    if (chdir(dir) == 0)
        getcwd(path, 100);
    else
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
    }

    return 0;
}

int myexit(int argc, char *argv[])
{
    if (argc == 2)
        exit(atoi(argv[1]));
    else
        exit(0);
}

Cmd *CmdCreate()
{
    Cmd *newcmd = (Cmd*)malloc(sizeof(Cmd));
    newcmd->argv = (char**)malloc(sizeof(char*) * 50);
    newcmd->argv[0] = NULL;
    newcmd->argc = 0;
    newcmd->next = NULL;
    return newcmd;
}

void CmdDestroy(Cmd *p)
{
    if (p != NULL)
    {
        if (p->argv != NULL)
            free(p->argv);
        free(p);
    }
}

void CmdListDestroy(Cmd *p)
{
    if (p != NULL)
    {
        Cmd *n = p->next;

        if (p->argv != NULL)
            free(p->argv);
        free(p);

        CmdListDestroy(n);
    }
}

Cmd *tokenize(char string[])
{
    Cmd *commands = CmdCreate();
    Cmd *currentCmd = commands;
    int argNum = 0;
    char quoted = 0;    // 0 = not quoted, '\'' = single, '"' = double

    // saving this since adding null characters
    // will cause this to change
    int length = strlen(string);
    int stringIsStarted = 0;
    int stringIsTerminated = 1;

    int i;
    for (i = 0; i < length; i++)
    {
        if (string[i] == '\0')
            break;

        if (argNum >= 50)
        {
            fprintf(stderr, "Error: Too many arguments\n");
            CmdListDestroy(commands);
            return NULL;
        }

        // printf("on char: %c, argnum: %d\n", string[i], argNum);
        if (quoted == 0)
        {
            if (string[i] != '\'' && string[i] != '"' && string[i] != ' ' && string[i] != '|' && string[i] != '\n' && string[i] != '\0')
            {
                if (stringIsTerminated)
                {
                    currentCmd->argv[argNum++] = string + i;
                    stringIsTerminated = 0;
                    currentCmd->argv[argNum] = NULL;
                }
            }
            else
            {
                if (string[i] == '\'')
                {
                    quoted = '\'';
                    stringIsStarted = 0;
                }
                else if (string[i] == '"')
                {
                    quoted = '"';
                    stringIsStarted = 0;
                }
                else if (string[i] == '|')
                {
                    if (argNum == 0)
                    {
                        fprintf(stderr, "Error: Piping without command\n");
                        CmdListDestroy(commands);
                        return NULL;
                    }

                    currentCmd->argc = argNum + 1;
                    currentCmd->next = CmdCreate();
                    currentCmd = currentCmd->next;
                    argNum = 0;
                    stringIsTerminated = 1;
                }
                else
                    stringIsTerminated = 1;

                string[i] = '\0';
            }
        }
        else
        {
            // printf("%c is in quote %c\n", string[i], quoted);
            if (string[i] == quoted)
            {
                string[i] = '\0';
                quoted = 0;
                stringIsTerminated = 1;
                stringIsStarted = 0;
            }
            else if (stringIsStarted == 0)
            {
                currentCmd->argv[argNum++] = string + i;
                stringIsStarted = 1;
                // stringIsTerminated = 1;
                currentCmd->argv[argNum] = NULL;
            }
        }
    }

    if (quoted != 0)
    {
        fprintf(stderr, "Error: missmatched quotes\n");
        CmdListDestroy(commands);
        return NULL;
    }

    if (argNum == 0)
    {
        fprintf(stderr, "Error: Not enough arguments\n");
        CmdListDestroy(commands);
        return NULL;
    }

    if (currentCmd->argc != argNum)
        currentCmd->argc = argNum;

    return commands;
}

int (*builtinCommands[])(int argc, char **argv) = {
    [0] cd,
    [1] myexit,
};

int callprogram(char *argv[], int in[2], int out[2])
{
    pid_t cpid = fork();

    if (cpid == -1)
    {
        fprintf(stderr, "failed to fork on %s\n", argv[0]);
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
        //  printf("'%s' ", t->argv[i]);
        // printf("\n");

        if (execvp(argv[0], argv) == -1)
        {
            fprintf(stderr, "%s on execvp(%s,...)\n",
                strerror(errno), argv[0]);
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
            // printf("executed without any redirects\n");
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
    return -1;  // should never reach this
}

void pipecommands(Cmd *commands)
{
    pid_t cpid;
    int status;
    int fdArray[50][2];
    // int fd[2];
    // int *prevPipe = NULL;
    int i = 0;

    // for (i = 0; i < commands->count - 1; i++)
    // {
    //  printf("making pipe %d\n", i);
    //  pipe(fdArray[i]);
    // }

    Cmd *currentCmd = commands;
    for (; currentCmd != NULL; currentCmd = currentCmd->next)
    {
        int *pipeIn = NULL;
        int *pipeOut = NULL;

        if (i != 0)
            pipeIn = fdArray[i - 1];

        if (currentCmd->next != NULL)
        {
            pipe(fdArray[i]);
            pipeOut = fdArray[i];
            i++;
        }

        // printf("callprogram %s with pipeIn: %ld (pipeArray[%d]), pipeOut: %ld (pipeArray[%d])\n",
        //  commands->tokens[i]->argv[0], pipeIn, i - 1, pipeOut, i);
        callprogram(currentCmd->argv, pipeIn, pipeOut);

        if (pipeIn != NULL)
        {
            // printf("closing pipe %d\n", i);
            close(pipeIn[0]);
            close(pipeIn[1]);
        }
    }

    while ((cpid = wait(&status)) != -1)
        printf("child %d exits with %d\n", cpid, WEXITSTATUS(status));
}

int main(int argc, char **argv)
{
    char command[512];
    getcwd(path, 100);
    while (printf("%s$ ", path), fgets(command, 512, stdin))
    {
        Cmd *commands = tokenize(command);

        if (commands == NULL)
            continue;

        // Cmd *currentCmd = commands;
        // for (; currentCmd != NULL; currentCmd = currentCmd->next)
        // {
        //  printf("%d args for %s: ", currentCmd->argc, currentCmd->argv[0]);
        //  int i;
        //  for (i = 0; currentCmd->argv[i] != NULL; i++)
        //      printf("'%s' ", currentCmd->argv[i]);
        //  printf("\n");
        // }

        // start parsing through the command
        if (strcmp(commands->argv[0], "exit") == 0)
            builtinCommands[1](0, NULL);
        else if (strcmp(commands->argv[0], "cd") == 0)
            builtinCommands[0](commands->argc, commands->argv);
        else if (commands->next == NULL)
            callprogram(commands->argv, NULL, NULL);
        else
            pipecommands(commands);

        CmdListDestroy(commands);
    }
    printf("\n");
    exit(0);
}
