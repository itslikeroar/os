#include "myshell.h"
#define homeDirectory getenv("HOME")

int numOfArgs = 0;
int fullPathFlag = 0;
char* path = NULL;

typedef int (*functPtr) (int, char**);
functPtr myfunctPtr[3];

int numOfArguments(char** arg)
{
    int count = 0;
    while(arg[count] != NULL)
    {
        count++;
        if(count > 2)
            return -1;
    }
    return 2;
}
int isalphanum(char arg)
{
    if(isalpha(arg) || isdigit(arg)) //cat moby.txt |tr A-Z a-z|tr -C a-z '\n' |sed '/^$/d' |sort|uniq -c|sort -nr|sed 10q
        return 1;
    else
        return 0;
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

pipestruct tokenize(char* arg)
{
    pipestruct pstruct = (struct PipeStruct *)calloc(1,sizeof(struct PipeStruct));
    pstruct->args = (char***)calloc(100,sizeof(char**));
    char** tokenized = (char**)calloc(100000,sizeof(char*));
    char* string = (char*)calloc(1000,sizeof(char));
    pstruct->error = 0;
    pstruct->empty = 1;
    int i;
    int place = 0;
    int index = 0;
    int indexer = 0;
    int flag = 0;
    int spaceBeforePipeFlag = 0;
    int quoteDetected = 0;
    for(i = 0; i < strlen(arg) + 1; i++)
    {
        if(arg[i] == '\'' || arg[i] == '\"')
        {
            if(quoteDetected == 0)
                quoteDetected = 1;
            else
                quoteDetected = 0;
        }
        else if(arg[i] == '|' && quoteDetected == 0)
        {
            if(spaceBeforePipeFlag == 1)
            {
                tokenized[index] = NULL;
                pstruct->args[indexer] = tokenized;
                tokenized = NULL;
                tokenized = (char**)realloc(tokenized, sizeof(char*) * 1000);
                pstruct->isPipe = 1;
                index = 0;
                spaceBeforePipeFlag = 0;
                indexer++;
            }
            else
            {
                string[place + 1] = '\0';
                tokenized[index] = string;
                tokenized[index + 1] = NULL;
                pstruct->args[indexer] = tokenized;
                tokenized = NULL;
                tokenized = (char**)realloc(tokenized, sizeof(char*) * 1000);
                pstruct->isPipe = 1;
                index = 0;
                string = NULL;
                string = (char*)realloc(string, sizeof(char) * 1000);
                indexer++;
                place = 0;
            }
        }
        else if((arg[i] == '\t' || arg[i] == ' ') && quoteDetected == 0)
        {
            if(arg[i - 1] != '|')
            {
                string[place + 1] = '\0';
                tokenized[index] = string;
                string = NULL;
                string = (char*)realloc(string, sizeof(char) * 1000);
                place = 0;
                numOfArgs++;
                index++;
                spaceBeforePipeFlag = 1;
            }
        }
        else if(arg[i] == '\0' && quoteDetected == 0)
        {
            if(flag > 0)
                fullPathFlag = 1;
            string[place + 1] = '\0';
            tokenized[index] = string;
            tokenized[index + 1] = NULL;
            pstruct->args[indexer] = tokenized;
            pstruct->args[indexer + 1] = NULL;
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
            spaceBeforePipeFlag = 0;
            pstruct->empty = 0;
        }
    }
    if(quoteDetected == 1)
        pstruct->error = 1;

   // free(string);
    //free(tokenized);
    return pstruct;
}
int cdTokenize(char* arg)
{
    int i = 2;
    int flag = 0;
    int areSpaces = 0;
    while(arg[i] != '\0')
    {
        if(isalphanum(arg[i]))
        {
            areSpaces = 1;
            flag = 1;
        }
        i++;
    }
    if(areSpaces == 0)
        return 0;
    else
        return 1;
}
void myexit(int argc, char** argv)
{
    //printf("\nProcess: %d exited with status=0\n", getpid());
    exit(0);
}

void myPipe(char*** arg)
{
    static int count = 0;
    static int flag = 0;
    static char* statBuff = NULL;
    int pid, pid1, status, status1, n;
    int fd[2];	
    int fe[2];
    int count1 = count + 1;
    char** arg1 = arg[count];
    char** arg2 = arg[count1];
    //printf("arg1: %s %s %s %s\n", arg1[0],arg1[1], arg1[2]. arg1[3]);
    //printf("arg2: %s %s %s %s\n", arg2[0], arg2[1], arg2[2],arg[3]);
     char buf[1000000];
   // printf("arg1: %s   %s\n", arg1[0], arg1[1]);
    //printf("arg2: %s   %s\n", arg2[count1], arg2[1]);
    //char** ls = NULL;
    //char** wc = (char*[]){"./hello", NULL};
   // exit(0);
    
        pipe(fe);
        if((pid = fork()) == -1)
        printf("Fork() failed");
        else if(pid == 0)
        {
            pipe(fd);
            pid1 = fork();
            if(pid1 == 0)
            {
       // printf("arg2[0]: %s", args2[0]);
            
            
            
                dup2(fd[0], 0);
                close(fd[1]);
            
            
                dup2(fe[1], 1);
                close(fe[0]);
            

                if(count == 0)
                {
                    execvp(arg2[0], arg2);
                }
                else
                {
                    execvp(arg1[0], arg1);
                }

            }
            else
            {
                if(count == 0)
                {
                    dup2(fd[1], 1);
                    close(fd[0]); /* the parent does not need this end of the pipe */
                    execvp(arg1[0], arg1);
                }
                else
                {
                    write(fd[1], statBuff, strlen(statBuff));
                    close(fd[0]);
                   // printf("Waht %s\n", statBuff);
                    //count++;
                   // printf("count: %d\n", count);
                }
            }
        
            /*if(waitpid(pid1, &status1, WUNTRACED | WCONTINUED))
            {
                if(WIFEXITED(status1))
                {
                    printf("\nProcess: %d exited with status=%d\n", getpid(), WEXITSTATUS(status1));
                }
            }//perror(arg1[0]);*/
        }
        else
        {
            if(flag == 0)
            {
                count = 1;
                flag = 1;
            }
            
                dup2(fe[0],0);
                close(fe[1]);
                if ((n = read(fe[0], buf, 1024)) >= 0) {
                    buf[n] = 0;	//terminate the string
                    //printf("read %d bytes from the pipe: \"%s\"\n", n, buf);
                }
                
                count++;
            printf("count: %d\n", count);
                statBuff = buf;
                printf("%s\n", statBuff);
          
            if(arg[count] == NULL)
            {
                //printf("EHAFLSAJFKASFJD %d\n", count);
                while ((pid = wait(&status)) != -1)
                    fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));

            }
            else
            {  // printf("asfasdf %d\n", count);
                myPipe(arg);
            }
        //printf("yo  %s \n ", buf);
        /*
        if(waitpid(pid, &status, WUNTRACED | WCONTINUED))
        {
            if(WIFEXITED(status))
            {
                printf("\nProcess: %d exited with status=%d\n", getpid(), WEXITSTATUS(status));
            }
        }*/
        

        //printf("arg2[0]: %s", args1[0]);
        //dup2(fd[1], 1);
        //close(fd[0]);	 the parent does not need this end of the pipe
        //execvp(arg1[0], arg1);
        //perror(arg2[1]);
        }
    /*
        else
        {
            printf("what: %s", statBuff);
            pipe(fe);
            
            pid = fork();
            pipe(fd);
            if(pid == 0)
            {
            

               
                dup2(fd[0], 0);
                close(fd[1]);
                
                
                dup2(fe[1], 1);
                close(fe[0]);
                
                
                execvp(arg1[0], arg1);
            }
            else
            {
                write(fd[1], statBuff, strlen(statBuff) + 1);
                close(fd[0]);
                printf("Waht %s\n", statBuff);
                
            }
            
            if(waitpid(pid, &status1, WUNTRACED | WCONTINUED))
            {
                if(WIFEXITED(status1))
                {
                    printf("\nProcess: %d exited with status=%d\n", getpid(), WEXITSTATUS(status));
                }
            }//perror(arg1[0]);

                      dup2(fe[0],0);
                      close(fe[1]);
                      if ((n = read(fe[0], buf, 1024)) >= 0) {
                      buf[n] = 0;	//terminate the string
                      printf("read %d bytes from the pipe: \"%s\"\n", n, buf);
                      }
                      
                      statBuff = buf;
                      count++;
            printf("count: %d \n", count);
            printf("buf: %s\n", buf);
            
                      
            if(arg[count] == NULL)
            {
                printf("answer: %s\n", statBuff);
                while ((pid = wait(&status)) != -1)
                    fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
            }
            else
            {
                myPipe(arg);
            }
*/
    
    
            /*
    if(waitpid(pid, &status, WUNTRACED | WCONTINUED))
    {
        if(WIFEXITED(status))
        {
            printf("\nProcess: %d exited with status=%d\n", getpid(), WEXITSTATUS(status));
        }
    }*/
}

void invokeStuff(int argc, char**argv)
{
    int status;
    pid_t pid;
    pid = fork();
    chdir(path);
    printf("arg: %s\n", argv[0]);
    if(pid == -1)
    {
        printf("FORK FAILED");
    }
    if(pid == 0)
    {
        if(execvp(argv[0], argv) == -1)
        {
            printf("errno: %d", errno);
        }
        exit(1);
    }
    if(waitpid(pid, &status, WUNTRACED | WCONTINUED))
    {
        if(WIFEXITED(status))
        {
            printf("\nProcess: %d exited with status=%d\n", getpid(), WEXITSTATUS(status));
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
        if(dp == NULL)
        	printf("ERROR");
        chdir(path);
        while((entry = readdir(dp)) != NULL)
        {
            lstat(entry->d_name, &statbuf);
            
            if(strcmp(argv[1],entry->d_name) == 0)
            {
                flag = 1;
                result = concat(newPath, concat("/", entry->d_name));
                chdir(result);
                break;
            }
        }
        if(flag == 1)
        {
            path = result;
            dp = opendir(path);
            //free(result);
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
    pipestruct ptr;
    chdir(homeDirectory);
    path = homeDirectory;
    char ch;
    while (1)
    {
        char command[200];
        printf("%s $: ",path);
        if((fgets(command, 200, stdin)) == NULL)
        {
            printf("\n");
            exit(0);
        }
        command[strlen(command) - 1] = '\0';
        printf("command: %s\n", command);
        ptr = tokenize(command);
        if(ptr->empty == 1)
        {
            
        }
        else if(ptr->error == 1)
        {
            printf("Mismatched quotes\n");
        }
        else if(ptr->isPipe == 1)
        {
            int i = 0;
            int j = 0;
            for(i = 0; ptr->args[i] != NULL; i++)
            {
                char** mychar = ptr->args[i];
                printf("prog %s ", mychar[0]);
                for(j = 1; mychar[j] != NULL; j++)
                {
                    printf("%s ", mychar[j]);
                }
                printf("\n");
            }
            //exit(0);
            myPipe(ptr->args);
        }
        else
        {
            if(command[0] == '.' && command[1] == '/')
            {
                ptr = tokenize(command);
                invokeStuff(1, ptr->args[0]);
                free(ptr->args);
                free(ptr);
            }
            else if(command[0] == 'c' && command[1] == 'd')
            {
                ptr = tokenize(command);
                if(cdTokenize(command) == 0)
                {
                    path = homeDirectory;
                    chdir(homeDirectory);
                }
                else if(numOfArguments(ptr->args[0]) == 2)
                {
                    myfunctPtr[1](numOfArgs, ptr->args[0]);
                    free(ptr->args);
                }
                else
                {
                    printf("cd: too many arguments.\n");
                }
                free(ptr);
            }
            else if(strcmp(command, "exit") == 0)
                myfunctPtr[0](0, NULL);
            else
            {
                //ptr = tokenize(command);
                if(ptr != NULL)
                {
                    invokeStuff(1, ptr->args[0]);
                    free(ptr->args);
                    free(ptr);
                }
            }
        }
    }
    return 0;
}