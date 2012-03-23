FILE_NAMES := $(wildcard *.cpp)
OBJECT_FILES := ${FILE_NAMES:.cpp=.o}
#CXX = gcc
CPP = g++
DEBUG = -g
CFLAGS = -Wall -pedantic $(DEBUG)
LDFLAGS = 
ALLEGROFLAGS = `allegro-config --cflags --libs`
MACROS = 
OBJS = position.o map.o player.o

all: test 

test: $(OBJS) test.o
	$(CPP) $(OBJS) test.o $(ALLEGROFLAGS) -o test.out

%.o: %.cpp
	$(CPP) $(ALLEGROFLAGS) -c $^ -o $@

.PHONY: clean

clean :
	rm -f *.o *~ test


