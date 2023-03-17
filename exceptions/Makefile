CC = gcc
CPP = g++
AS = as
CFLAGS = -O2
LFLAGS = -lm
PFLAGS = -pthread

all: build main.elf
amd: build main-amd.elf

build: 
	mkdir build

main.elf: main.cpp gates/single.h build/compose.o
	$(CPP) $(CFLAGS) -o main.elf main.cpp build/compose.o $(LFLAGS)

build/compose.o: gates/compose.cpp gates/compose.h gates/single.h gates/gates.h
	$(CPP) $(CFLAGS) -D INTEL -c -o build/compose.o gates/compose.cpp

main-amd.elf: main.cpp gates/single.h build/compose-amd.o
	$(CPP) $(CFLAGS) -o main-amd.elf main.cpp build/compose-amd.o $(LFLAGS)

build/compose-amd.o: gates/compose.cpp gates/compose.h gates/single.h gates/gates.h
	$(CPP) $(CFLAGS) -c -o build/compose-amd.o gates/compose.cpp

clean:
	rm -rf *.o build/ *.elf *~
