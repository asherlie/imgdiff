CC=gcc
CFLAGS= -Wall -Wextra -Wpedantic -Werror -lz

all: imgd 


imgd: imgd.c
	$(CC) $(CFLAGS) imgd.c -o imgd

.PHONY:

clean:
	rm -f imgd *.o
