CC = gcc
CFLAGS = -Wall -g -O0 
SRCS = $(wildcard *.c)
#EXECS = $(patsubst %.c,%,$(SRCS))
EXECS = client server #client2

all: $(EXECS) 

client: file_base.h common.h 
server: communicate.h common.h channels_base.h handle_msg.h
#client2: msg_struct.h common.h

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f client server