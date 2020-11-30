CC       = gcc
ARCH	 = amd64
INCL     = -I kernel/include -I kernel/libk/include -I arch -I arch/$(ARCH)
CFLAGS   = -c -std=c11 -Wall -g -O0 -D DEBUG \
           -ffreestanding -nostdinc -fno-strict-aliasing \
           -fno-builtin -fno-stack-protector -mno-red-zone \
	   $(INCL) -DARCH=$(ARCH) -fvar-tracking
LDFLAGS  = -nostdlib --warn-common -nmagic --no-relax

BOOT32_OBJ = $(patsubst %.c, %32.o, $(wildcard boot/*.c)) \
             $(patsubst %.S, %32.o, $(wildcard boot/*.S)) \
             kernel.o

KERNEL_OBJ = $(patsubst %.S, %.o, $(wildcard kernel/*/*.S arch/$(ARCH)/*.S)) \
             $(patsubst %.c, %.o, $(wildcard kernel/*/*.c arch/$(ARCH)/*.c \
             kernel/*.c)) \

all: boot32 diskimage

meta:
	@ echo " GEN generating version information"
	@ tools/version.sh

%32.o: %.c
	@ echo "CC32 $<"
	@ $(CC) $< -m32 $(CFLAGS) -DM32 -o $@

%32.o: %.S
	@ echo "AS32 $<"
	@ $(CC) $< -m32 $(CFLAGS) -o $@

%.o: %.S
	@ echo " AS  $<"
	@ $(CC) $< $(CFLAGS) -o $@
	
%.o: %.c meta
	@ echo " CC  $<"
	@ $(CC) $< $(CFLAGS) -o $@

boot32: $(BOOT32_OBJ)
	@ echo " LD  vmulmer"
	@ $(LD) -T boot/boot32.ld $(LDFLAGS) $(BOOT32_OBJ) -o vmulmer

kernel.o: $(KERNEL_OBJ) meta
	@ echo " LD  kernel"
	@ $(LD) -T kernel/kernel.ld $(LDFLAGS) $(KERNEL_OBJ) -o kernel64
	@ objcopy -O binary kernel64 kernel64.bin
	@ objcopy -I binary -O elf32-i386 \
		-B i386 kernel64.bin kernel.o

diskimage: boot32 kernel.o
	@ echo " GEN generating disk image"
	@ tools/mkimg.sh > /dev/null 2> /dev/null

qemu:
	@ tools/qemu.sh
