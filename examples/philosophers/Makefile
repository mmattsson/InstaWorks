# ---
#
# Makefile for philosophers example 
#
# Copyright (C) Mattias Mattsson - 2014
#
# ---

BIN=philosophers
OBJS=main.o
CFLAGS=-g -O0 -I../../includes -Wall -Werror
LDFLAGS=-rdynamic -L../../bin -linstaworks -lpthread

.PHONY: clean

($BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS) 

clean:
	rm -f $(BIN) *.o *~

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# ---

