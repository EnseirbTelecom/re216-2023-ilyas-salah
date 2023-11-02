CC = gcc
CFLAGS = -Wall -g -O0 
SRCS = $(wildcard *.c)
#EXECS = $(patsubst %.c,%,$(SRCS))
EXECS = client server

all: $(EXECS) 

client: communicate.h common.h
server: communicate.h common.h

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f client server