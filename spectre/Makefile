CFLAGS += -std=c99 -O0

PROGRAM = spectre.elf
SOURCE  = spectre.c
     
all: $(PROGRAM)
     
$(PROGRAM): $(SOURCE) ; $(CC) $(CFLAGS) -o $(PROGRAM) $(SOURCE)
     
clean: ; rm -f $(PROGRAM)