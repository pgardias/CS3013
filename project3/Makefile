# Author: pmgardias
CC=gcc
CFLAGS=-g -Wall

all: bathroomSim.c bathroom.c bathroom.h
	$(CC) $(CFLAGS) bathroomSim.c bathroom.c bathroom.h -o sim -pthread -lm

clean:
	rm -f sim *.o *.exe

