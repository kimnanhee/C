#
# Copyright 2012. Antonino N. Mione
#
# makefile for procInfoServer
#

ifndef SERVER_PORT
CFLAGS=-ggdb -O0 -DSERVER_PORT=7060
else
CFLAGS=-ggdb -O0 -DSERVER_PORT=$(SERVER_PORT)
endif

CC=gcc
LD=gcc

procInfoServer: procInfoServer_main.o procInfoServer_auth.o procInfoServer_util.o procInfoServer_commands.o procInfoServer_signals.o
	gcc -o procInfoServer procInfoServer_main.o procInfoServer_auth.o procInfoServer_util.o procInfoServer_commands.o procInfoServer_signals.o

procInfoServer_main.o: procInfoServer_main.c

procInfoServer_auth.o: procInfoServer_auth.c

procInfoServer_util.o: procInfoServer_util.c

procInfoServer_commands.o: procInfoServer_commands.c

procInfoServer_signals.o: procInfoServer_signals.c


clean:
	rm procInfoServer procInfoServer_auth.o procInfoServer_main.o procInfoServer_util.o procInfoServer_commands.o procInfoServer_signals.o
