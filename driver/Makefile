CC=gcc
CFLAGS=-I.

all: malinkabtn malinkabtn-sans

malinkabtn: malinkabtn.o
	$(CC) -o malinkabtn malinkabtn.o -g -Wall -pthread -l bcm2835 -lrt
malinkabtn-sans: malinkabtn-sans.o
	$(CC) -o malinkabtn-sans malinkabtn-sans.o -g -Wall -pthread -l bcm2835 -lrt
