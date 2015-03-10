#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#define homeDirectory getenv("HOME")

int numOfArgs = 0;
int fullPathFlag = 0;
char* path = NULL;

typedef int (*functPtr) (int, char**);
functPtr myfunctPtr[3];

char** tokenize(char* arg)
{
    char** tokenized = (char**)malloc(sizeof(char*) * 1000000);
    char* string = (char*)malloc(sizeof(char) * 1000);
    int i;
    int place = 0;
    int index = 0;
    int flag = 0;
    for(i = 0; i < strlen(arg) + 1; i++)
    {
        if(arg[i] == ' ')
        {
            string[place + 1] = '\0';
            tokenized[index] = string;
            string = NULL;
            string = (char*)realloc(string, sizeof(char) * 1000);
            place = 0;
            numOfArgs++;
            index++;
        }
        else if(arg[i] == '\0')
        {
            if(flag > 0)
                fullPathFlag = 1;
            string[place + 1] = '\0';
            tokenized[index] = string;
            tokenized[index + 1] = NULL;
            string = NULL;
            string = (char*)realloc(string, sizeof(char) * 1000);
            numOfArgs++;
            flag = 0;
            continue;
        }
        else
        {
            if(arg[i] == '/')
                flag++;
            string[place] = arg[i];
            place++;
        }
    }
    free(string);
    return tokenized;
}

char* concat(char *s1, char *s2)
{
    //have to free this somehow
    
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
void myexit(int argc, char** argv)
{
    exit(0);
}

void invokeStuff(int argc, char**argv)
{
    int status;
    pid_t pid;
    pid = fork();
    chdir(path);
    if(pid == -1)
    {
        printf("FORK FAILED");
    }
    if(pid == 0)
    {
        printf("%s sdafkljsdf; ", argv[1]);
            if(execvp(argv[0], argv) == -1)
           {
               printf("errno: %d", errno);
           }
    }
    if(waitpid(pid, &status, WUNTRACED | WCONTINUED))
    {
        if(WIFEXITED(status))
        {
            printf("\nChild exited, status=%d\n", WEXITSTATUS(status));
        }
    }
}

void mycd(int argc, char** argv)
{
    short flag = 0;
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char* result;
    char* newPath = path;
    if(fullPathFlag == 1)
    {
        newPath = argv[1];
        dp = opendir(newPath);
        if(dp == NULL)
            printf("ERROR");
        else
        {
            chdir(path);
            path = newPath;
            fullPathFlag = 0;
        }
    }
    else
    {
        dp = opendir(path);
        chdir(path);
        while((entry = readdir(dp)) != NULL)
        {
            lstat(entry->d_name, &statbuf);
            
            if(strcmp(argv[1],entry->d_name) == 0)
            {
                flag = 1;
                result = concat(newPath, concat("/", entry->d_name));
                chdir(newPath);
                break;
            }
        }
        if(flag == 1)
        {
            path = result;
            dp = opendir(path);
        }
        else
        {
            printf("ERROR!\n");
        }
    }
}
int main(int argc, char** argv)
{
    myfunctPtr[0] = (functPtr)myexit;
    myfunctPtr[1] = (functPtr)mycd;
    //myfunctPtr[2] = (functPtr)myls;
    char** args = NULL;
    char* lsArgs[3];
    chdir(homeDirectory);
    path = homeDirectory;
    while(1)
    {
        char command[200];
        printf("%s: ",path);
        scanf("%[^\n]s", command);
        if(command[0] == '.' && command[1] == '/')
        {
            args = tokenize(command);
            invokeStuff(1, args);
            while (getchar() != '\n');
            free(args);
        }
        else if(strcmp(command, "ls") == 0)
        {
            lsArgs[0] = "ls";
            lsArgs[1] = "-1";
            lsArgs[2] = NULL;
            invokeStuff(3, lsArgs);
            while (getchar() != '\n' );
        }
        else if(command[0] == 'c' && command[1] == 'd')
        {
            args = tokenize(command);
            if(command[2] == '\0')
            {
                path = homeDirectory;
                chdir(homeDirectory);
            }
            else
            {
                myfunctPtr[1](numOfArgs, args);
            }
            free(args);
            while (getchar() != '\n' );
        }
        else if(strcmp(command, "exit") == 0)
        myfunctPtr[0](0, NULL);
        else
        {
            args = tokenize(command);
            invokeStuff(1, args);
            while (getchar() != '\n');
            free(args);
        }
    }
    return 0;
}