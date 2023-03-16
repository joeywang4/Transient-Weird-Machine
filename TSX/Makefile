CC = gcc
CPP = g++
AS = as
CFLAGS = -O2
LFLAGS = -lm
PFLAGS = -pthread

all: tsx.elf

tsx.elf: tsx.c
	$(CC) $(CFLAGS) -o tsx.elf tsx.c $(LFLAGS)

clean:
	rm -f *.o tsx.elf *~
