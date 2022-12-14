CC = g++
CFLAGS = -g -Wall -lfinal 
SRCS = $(wildcard *.cpp)

PROGS = $(patsubst %.cpp,%,$(SRCS))

all: $(PROGS)

%: %.cpp

	$(CC) $(CFLAGS)  -o $@ $<


clean: $(PROGS)
	rm $(PROGS)
