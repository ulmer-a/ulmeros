CC = /usr/local/cross/bin/x86_64-elf-gcc
LD = /usr/local/cross/bin/x86_64-elf-ld
USERLD = ../arch/amd64.ld

init_OBJ = init.o

all: init

init: $(init_OBJ)
	$(LD) -T $(USERLD) -z max-page-size=4096 $(init_OBJ) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
