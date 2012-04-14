FILE_NAMES := $(wildcard *.cpp)
OBJECT_FILES := ${FILE_NAMES:.cpp=.o}
#CXX = gcc
CPP = g++
DEBUG = -g
CFLAGS = -Wall -pedantic $(DEBUG)
LDFLAGS = #-L/home/prashantv/usr/local/lib
ALLEGROFLAGS = `allegro-config --cflags --libs`
MACROS = 
OBJS = globals.o position.o map.o player.o packet.o chat.o mask.o

all: client server

test: $(OBJS) test.o
	$(CPP) $(OBJS) test.o $(LDFLAGS) $(ALLEGROFLAGS) -o test.out

client: $(OBJS) client.o 
	$(CPP) $(OBJS) client.o $(LDFLAGS) $(ALLEGROFLAGS) -o client.out

server: $(OBJS) server.o
	$(CPP) $(OBJS) server.o $(LDFLAGS) $(ALLEGROFLAGS) -o server.out

map.o: map.cpp mask.h constants.h
	$(CPP) $(ALLEGROFLAGS) $(LDFLAGS) -c $< -o $@

%.o: %.cpp constants.h
	$(CPP) $(ALLEGROFLAGS) $(LDFLAGS) -c $< -o $@

.PHONY: clean

clean :
	rm -f *.o *~ test


