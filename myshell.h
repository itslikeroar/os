#ifndef MYSHELL_H
#define MYSHELL_H

typedef struct token {
	int argc;
	char **argv;
	int isPipe;
} Token;


#endif