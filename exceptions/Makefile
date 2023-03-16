CC = gcc
CPP = g++
AS = as
CFLAGS = -O2
LFLAGS = -lm
PFLAGS = -pthread

all: build main.elf

build: 
	mkdir build

main.elf: main.cpp gates/single.h build/compose.o
	$(CPP) $(CFLAGS) -o main.elf main.cpp build/compose.o $(LFLAGS)

build/compose.o: gates/compose.cpp gates/compose.h gates/single.h gates/gates.h
	$(CPP) $(CFLAGS) -c -o build/compose.o gates/compose.cpp

clean:
	rm -rf *.o build/ main.elf *~
