CC = gcc
CFLAGS = -Wall -g
# OBJS =
all:	shell

shell:	shell.c
	$(CC) $(CFLAGS) shell.c -o shell

clean:
	rm -rf shell