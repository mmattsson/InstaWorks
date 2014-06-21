# ---
#
# Simple example makefile
#
# Copyright (C) Mattias Mattsson - 2014
#
# ---

BIN=simple
OBJS=main.o
CFLAGS=-g -O0 -I../../includes
LDFLAGS=-L../../bin -linstaworks -lpthread

.PHONY: clean

($BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS) 

clean:
	rm -f $(BIN) *.o *~

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# ---

