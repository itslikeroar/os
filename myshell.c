#include <stdio.h>
#include "myshell.h"

int cd(int argc, char **argv)
{
	return 0;
}

int exit(int argc, char *argv[])
{
	if (argc == 2)
		exit(atoi(argv[1]));
	else
		exit(0);
}

int main(int argc, char **argv)
{
	while (1)
	{
		
	}
	exit(0);
}
