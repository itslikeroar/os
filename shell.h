/*
*
* Operating Systems: CS416 Assignment 5
* 
* Group Members:
* Rodrigo Pacheco	rap256
* Brian Yoo			bgy2
* Sisheng Zhang		sz320
* 
*/

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
#ifndef SHELL_H
#define SHELL_H

// holds information necessary to run one or more commands
typedef struct command
{
	char **argv;
	int argc;
	struct command *next;
} Cmd;

#endif