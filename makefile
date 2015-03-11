CC = gcc
CFLAGS = -Wall -g
# OBJS =
all:	myshell

myshell:	myshell.c
	$(CC) $(CFLAGS) myshell.c -o myshell

clean:
	rm -rf myshell