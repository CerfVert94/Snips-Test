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

snips : snips.o topic-regex.o
	${CC} $(CFLAG) $(INCLUDE) $^ -o $@ $(LIBS)

topic-regex : topic-regex.c
	${CC} $(CFLAG) $(INCLUDE) $^ -o $@ -DDEBUG_MODE

snips.o :snips.c 
	${CC} $(CFLAG) $(INCLUDE)   -c $< -o $@ $(DFLAG)

topic-regex.o : topic-regex.c 
	${CC} $(CFLAG) $(INCLUDE) -c $< -o $@ $(DFLAG)

clean : 
	-rm -f *.o snips topic-regex
