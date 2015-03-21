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

#ifndef SHELL_H
#define SHELL_H

typedef struct command
{
	char **argv;
	// int argc;
	struct command *next;
} Cmd;

#endif