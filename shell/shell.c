/*
*
* Operating Systems: CS416 Assignment 5
* 
* Group Members:
* Rodrigo Pacheco   rap256
* Brian Yoo         bgy2
* Sisheng Zhang     sz320
* 
*/
#include "shell.h"
#define homeDirectory getenv("HOME")
char path[100];

/**
 * cd will change to a given directory
 * or if it is just called without arguments,
 * it will change to homeDirectory
 * if the given directory cannot be changed to,
 * it will print out an error
 */
int cd(int argc, char **argv)
{
    char *dir = NULL;
    if (argc == 1)
        dir = homeDirectory;
    else if (argc == 2)
        dir = argv[1];
    else
    {
        fprintf(stdout, "cd: Too many arguments.\n");
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

/**
 * myexit will call exit with 0 if no arguments are given,
 * if one argument is passed, it will call exit with
 * the argument converted to an int and if there are more
 * arguments, it will print out an error stating that there
 * are too many arguments
 */
int myexit(int argc, char *argv[])
{
    if (argc == 1)
        exit(0);
    else if (argc == 2)
        exit(atoi(argv[1]));
    else
    {
        fprintf(stdout, "exit: Too many arguments.\n");
        return 1;
    }
}

/**
 * mallocs and initializes the space needed for a Cmd struct
 * returns the malloc'd struct
 */
Cmd *CmdCreate()
{
    Cmd *newcmd = (Cmd*)malloc(sizeof(Cmd));
    newcmd->argv = (char**)malloc(sizeof(char*) * 50);
    newcmd->argv[0] = NULL;
    newcmd->argc = 0;
    newcmd->next = NULL;
    return newcmd;
}

/**
 * frees the memory associated with a Cmd struct and
 * follows the next pointers recursively to free up
 * all the memory of a Cmd list
 */
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

/**
 * tokenize will take a string array and return a Cmd struct list
 * each Cmd struct will hold a null terminated array of pointers
 * to the original string which will serve as argv
 * if there are more than one Cmd structs in the list, then the
 * commands will be piped
 */
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

        if (quoted == 0)
        {
            if (string[i] != '\'' && string[i] != '"' && string[i] != ' ' && string[i] != '\t' && string[i] != '|' && string[i] != '\n' && string[i] != '\0')
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

                    currentCmd->argc = argNum;
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
        if (commands->next != NULL)
        {
            fprintf(stderr, "Error: Not enough arguments\n");
            CmdListDestroy(commands);
        }
        return NULL;
    }

    if (currentCmd->argc != argNum)
        currentCmd->argc = argNum;

    return commands;
}

// array of function pointers for built-in commands
int (*builtinCommands[])(int argc, char **argv) = {
    [0] cd,
    [1] myexit,
};

/**
 * call program will run a program using the argv passed
 * the in and out pointers serve as pointers to pipes which
 * these should be set to null if no piping is required
 * if the input needs to be read from a pipe, in should be set
 * and if the output needs to written to a pipe, out should be set
 * if either is set, the parent will not wait to allow for the
 * programs to run concurrently
 */
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
            close(in[1]);
            dup2(in[0], 0);
            close(in[0]);
        }

        if (out != NULL)
        {
            close(out[0]);
            dup2(out[1], 1);
            close(out[1]);
        }

        if (execvp(argv[0], argv) == -1)
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));        
        // error if it reaches this
        exit(1);
    }
    else
    {
        // parent
        if (in == NULL && out == NULL)
        {
            int exitstatus;
            wait(&exitstatus);
            exitstatus = WEXITSTATUS(exitstatus);
            printf("process %d exits with %d\n", cpid, exitstatus);
            return exitstatus;
        }
        else
            return 0; // don't wait yet
    }
    return -1;  // should never reach this
}

/**
 * pipecommands pipe commands that will be read from a
 * list of Cmd structs
 */
void pipecommands(Cmd *commands)
{
    pid_t cpid;
    int status;
    int fdArray[50][2];
    int i = 0;

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

        callprogram(currentCmd->argv, pipeIn, pipeOut);

        if (pipeIn != NULL)
        {
            close(pipeIn[0]);
            close(pipeIn[1]);
        }
    }

    while ((cpid = wait(&status)) != -1)
        printf("process %d exits with %d\n", cpid, WEXITSTATUS(status));
}

// main will only give a promt to the user if stdin is a terminal
int main(int argc, char **argv)
{
    char command[512];
    getcwd(path, 100);

    while (1)
    {
        if (isatty(0))  // check if stdin is terminal
            printf("%s$ ", path);

        if (fgets(command, 512, stdin) == NULL)
            break;

        Cmd *commands = tokenize(command);

        if (commands == NULL)
            continue;

        // start parsing through the command
        if (strcmp(commands->argv[0], "exit") == 0)
            builtinCommands[1](commands->argc, commands->argv);
        else if (strcmp(commands->argv[0], "cd") == 0)
            builtinCommands[0](commands->argc, commands->argv);
        else if (commands->next == NULL)
            callprogram(commands->argv, NULL, NULL);
        else
            pipecommands(commands);

        CmdListDestroy(commands);
    }
    if (isatty(0))
        printf("\n");
    exit(0);
}
