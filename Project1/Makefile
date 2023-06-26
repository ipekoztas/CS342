CC = gcc
CFLAGS = -Wall -pthread

all: threadtopk proctopk

threadtopk: threadtopk.c
	$(CC) $(CFLAGS) -o threadtopk threadtopk.c

proctopk: proctopk.c
	$(CC) $(CFLAGS) -o proctopk proctopk.c

clean:
	rm -f threadtopk proctopk

