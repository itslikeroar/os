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

struct PipeStruct
{
    char*** args;
    int isPipe;
    int error;
    int empty;
};
typedef struct PipeStruct *pipestruct;



