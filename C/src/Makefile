CC=gcc
CFLAG=-g -Wall
SOVERSION=1
LIB_DIR=../lib
INCLUDE=-I. -I$(LIB_DIR) -L$(LIB_DIR) $(LIB_DIR)/libmosquitto.so.$(SOVERSION)
LIBS=-lmosquitto -lm -lrt -lssl -lcrypto -lcares -lpthread
VERSION=1.4.8
DFLAG=-DVERSION="\"${VERSION}\""

.PHONY: all clean

all : snips snips-regex

snips : snips.o 
	${CC} $(CFLAG) $(INCLUDE) $^ -o $@ $(LIBS)
snips-regex : snips-regex.o 
	${CC} $(CFLAG) $(INCLUDE) $^ -o $@ 

snips.o :snips.c $(LIB_DIR)/libmosquitto.so.$(SOVERSION)
	${CC} $(CFLAG) $(INCLUDE)   -c $< -o $@ $(DFLAG)

snips-regex.o : snips-regex.c $(LIB_DIR)/libmosquitto.so.$(SOVERSION)
	${CC} $(CFLAG) $(INCLUDE) -c $< -o $@ $(DFLAG)

clean : 
	-rm -f *.o snips snips-regex
