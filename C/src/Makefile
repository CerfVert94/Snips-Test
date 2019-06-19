CC=gcc
CFLAG=-g -Wall
SOVERSION=1
LIB_DIR=../lib
INCLUDE=-I. -I$(LIB_DIR) -L$(LIB_DIR)
LIBS=-lmosquitto -lm -lrt -lssl -lcrypto -lcares -lpthread
VERSION=1.4.8
DFLAG=-DVERSION="\"${VERSION}\""

.PHONY: all clean

all : snips

snips : snips.o snips-regex.o
	${CC} $(CFLAG) $(INCLUDE) $^ -o $@ $(LIBS)

snips-regex : snips-regex.c
	${CC} $(CFLAG) $(INCLUDE) $^ -o $@ -DDEBUG_MODE

snips.o :snips.c 
	${CC} $(CFLAG) $(INCLUDE)   -c $< -o $@ $(DFLAG)

snips-regex.o : snips-regex.c 
	${CC} $(CFLAG) $(INCLUDE) -c $< -o $@ $(DFLAG)

clean : 
	-rm -f *.o snips snips-regex
