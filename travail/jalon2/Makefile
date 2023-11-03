CC = gcc
CFLAGS = -Wall -g -O0 
SRCS = $(wildcard *.c)
#EXECS = $(patsubst %.c,%,$(SRCS))
EXECS = client server #client2

all: $(EXECS) 

client: msg_struct.h common.h
server: communicate.h common.h
#client2: msg_struct.h common.h

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f client server