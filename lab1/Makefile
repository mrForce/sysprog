# This is a comment line

CFLAGS=-g -Wall -std=c99

all : xorbit reversebytes

# note the hard tab at the beginning of the cc line 
xorbit : xorbit.c
	cc $(CFLAGS) -o xorbit xorbit.c

reversebytes : reversebytes.c
	cc $(CFLAGS) -o reversebytes reversebytes.c 

clean: 
	rm -rf *.o *~ xorbit reversebytes
