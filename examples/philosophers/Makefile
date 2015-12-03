# ---
#
# Makefile for philosophers example 
#
# Copyright (c) 2014 Mattias Mattsson. All rights reserved.
# This source is distributed under the license in LICENSE.txt in the top
# InstaWorks directory.
#
# ---

BIN=philosophers
OBJS=main.o
CFLAGS=-g -O0 -I../../includes -Wall -Werror
LDFLAGS=-rdynamic -L../../lib -linstaworks -lpthread

.PHONY: clean

($BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS) 

clean:
	rm -f $(BIN) *.o *~

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# ---

