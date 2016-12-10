CC=g++
STD=-std=c++14


all: clean sim

sim: main.o cache.o memory.o 
	$(CC) $(STD) -o $@ $^  

main.o: storage.h cacti/io.h

cache.o: cache.h def.h cache.cc

memory.o: memory.h memory.cc 

.PHONY: clean

clean:
	rm -rf sim *.o
