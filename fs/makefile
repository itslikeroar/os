CC = gcc
CFLAGS = -Wall -g
all:	mathfs

mathfs:	mathfs.c mathfs.h
	$(CC) $(CFLAGS) mathfs.c `pkg-config fuse --cflags --libs` -o mathfs -lm

clean:
	rm -rf mathfs
