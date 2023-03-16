CC = gcc

all: arm.elf

arm.elf: signal.c gates.s
	$(CC) -o arm.elf signal.c gates.s -lpthread

clean:
	rm arm.elf
