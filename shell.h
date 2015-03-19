#ifndef MYSHELL_H
#define MYSHELL_H

typedef struct token {
	int argc;
	char **argv;
	// int isPipe;
} Token;

typedef struct commands {
	Token **tokens;
	int count;
} Cmds;

#endif