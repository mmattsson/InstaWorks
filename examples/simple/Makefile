# ---
#
# Simple example makefile
#
# Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
# This source is distributed under the license in LICENSE.txt in the top
# InstaWorks directory.
#
# ---

BIN=simple
OBJS=main.o
CFLAGS=-g -O0 -I../../includes
LDFLAGS=-L../../lib -linstaworks -lpthread

.PHONY: clean

($BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS) 

clean:
	rm -f $(BIN) *.o *~

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# ---

