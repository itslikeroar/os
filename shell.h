#ifndef SHELL_H
#define SHELL_H

typedef struct command
{
	char **argv;
	int argc;
	struct command *next;
} Cmd;

#endif