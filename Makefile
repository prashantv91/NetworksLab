FILE_NAMES := $(wildcard *.cpp)
OBJECT_FILES := ${FILE_NAMES:.cpp=.o}
#CXX = gcc
CPP = g++
DEBUG = -g
CFLAGS = -Wall -pedantic $(DEBUG)
LDFLAGS = -L/home/arijit/usr/local/lib
ALLEGROFLAGS = `allegro-config --cflags --libs`
MACROS = 
OBJS = position.o map.o player.o packet.o client.o server.o globals.o

all: test 

test: $(OBJS) test.o
	$(CPP) $(OBJS) test.o $(LDFLAGS) $(ALLEGROFLAGS) -o test.out

%.o: %.cpp
	$(CPP) $(ALLEGROFLAGS) $(LDFLAGS) -c $^ -o $@

.PHONY: clean

clean :
	rm -f *.o *~ test


