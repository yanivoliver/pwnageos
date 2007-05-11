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
OUTPUT	= bin/
SRC 		= src/
INCLUDE = include/

# $@ - Output
# $? - Depends

kernel.bin : main.o kernel_lowlevel.o interrupts.o io.o screen.o irq.o memory.o gdt.o tss.o
	$(LD) -o $(OUTPUT)$@ --entry=0x1400 -Ttext 0x1400 --omagic -O 3 --oformat binary $?
	make clean
	make bootloader
	make boot.bin
	
bootloader: $(SRC)bootloader.asm
	$(NASM) -o $(OUTPUT)$@ $?
	
boot.bin: $(SRC)boot.asm
	$(NASM) -o $(OUTPUT)$@ $?

interrupts.o : $(SRC)interrupts.c
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)interrupts.c
	
io.o : $(SRC)io.c
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)io.c

irq.o : $(SRC)irq.c
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)irq.c

screen.o : $(SRC)screen.c io.o
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)screen.c
	
memory.o : $(SRC)memory.c
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)memory.c
	
gdt.o : $(SRC)gdt.c memory.o
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)gdt.c
	
tss.o : $(SRC)tss.c memory.o gdt.o
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)tss.c

main.o : $(SRC)main.c kernel_lowlevel.o interrupts.o screen.o io.o irq.o memory.o gdt.o tss.o
	$(CC) -c -I$(INCLUDE) -o $@ $(SRC)main.c

kernel_lowlevel.o : $(SRC)kernel_lowlevel.asm
	$(NASM) -f coff -I$(INCLUDE) -o $@ $?
	
clean:
	rm *.o
		
