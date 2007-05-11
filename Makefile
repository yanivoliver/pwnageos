#
# Makefile
# Author: Shimi G.
#

AS		= /usr/cross/i586-elf/bin/as.exe
LD		= /usr/cross/i586-elf/bin/ld.exe
CC		= /usr/cross/i586-elf/bin/gcc.exe
DBW		= dbw.exe
NASM	= nasmw
CFLAGS	= -Wall
OUTPUT	= output/

# $@ - Output
# $? - Depends

kernel.bin : main.o kernel_lowlevel.o interrupts.o io.o screen.o irq.o memory.o gdt.o tss.o
	$(LD) -o $(OUTPUT)$@ --entry=0x1400 -Ttext 0x1400 --omagic -O 3 --oformat binary $?
	make clean
	make bootloader
	make boot.bin
	
bootloader: bootloader.asm
	$(NASM) -o $(OUTPUT)$@ $?
	
boot.bin: boot.asm
	$(NASM) -o $(OUTPUT)$@ $?

interrupts.o : interrupts.c interrupts.h common.h irq.h
	$(CC) -c -o $@ interrupts.c
	
io.o : io.c io.h common.h
	$(CC) -c -o $@ io.c

irq.o : irq.c irq.h io.h common.h interrupts.h
	$(CC) -c -o $@ irq.c

screen.o : screen.c screen.h io.h io.o common.h
	$(CC) -c -o $@ screen.c
	
memory.o : memory.c common.h
	$(CC) -c -o $@ memory.c
	
gdt.o : gdt.c memory.o common.h gdt.h
	$(CC) -c -o $@ gdt.c
	
tss.o : tss.c memory.o gdt.o
	$(CC) -c -o $@ tss.c

main.o : main.c kernel_lowlevel.o interrupts.o screen.o io.o irq.o memory.o gdt.o tss.o common.h
	$(CC) -c -o $@ main.c

kernel_lowlevel.o : kernel_lowlevel.asm
	$(NASM) -f coff -o $@ $?
	
clean:
	rm *.o
		
