CC = gcc
CFLAGS = -Wall -g -O0 
SRCS = $(wildcard *.c)
#EXECS = $(patsubst %.c,%,$(SRCS))
EXECS = client

all: $(EXECS) 



%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	@-/bin/rm -f *.o *~  *.out core $(EXECS) 