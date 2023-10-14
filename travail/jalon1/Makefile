CC = gcc
CFLAGS = -Wall -g -O0 
SRCS = $(wildcard *.c)
#EXECS = $(patsubst %.c,%,$(SRCS))
EXECS = client server

all: $(EXECS) 

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f client server