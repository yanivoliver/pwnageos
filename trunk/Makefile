#
# Makefile
# Author: Shimi G.
#

AS = /usr/cross/i586-elf/bin/as.exe
LD = /usr/cross/i586-elf/bin/ld.exe
CC = /usr/cross/i586-elf/bin/gcc.exe
NASM	= nasmw
CFLAGS	= -Wall -nostdlib -fno-builtin
OUTPUT	= bin/
SRC 		= src/
INCLUDE = include/

# $@ - Output
# $? - Depends

kernel.bin : main.o kernel_lowlevel.o interrupts.o io.o syscall.o screen.o irq.o memory.o gdt.o tss.o schedule.o string.o keyboard.o
	$(LD) -o $(OUTPUT)$@ --entry=0x1400 -Ttext 0x1400 --omagic -O 3 --oformat binary $?
	make clean
	make bootloader
	make boot.bin
	
bootloader: $(SRC)bootloader.asm
	$(NASM) -o $(OUTPUT)$@ $?
	
boot.bin: $(SRC)boot.asm
	$(NASM) -o $(OUTPUT)$@ $?

screen.o : $(SRC)screen.c io.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)screen.c
	
memory.o : $(SRC)memory.c
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)memory.c
	
gdt.o : $(SRC)gdt.c memory.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)gdt.c
	
tss.o : $(SRC)tss.c memory.o gdt.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)tss.c
	
schedule.o : $(SRC)schedule.c
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)schedule.c

interrupts.o : $(SRC)interrupts.c tss.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)interrupts.c
	
io.o : $(SRC)io.c
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)io.c

string.o : $(SRC)string.c
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)string.c

irq.o : $(SRC)irq.c tss.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)irq.c
	
keyboard.o : $(SRC)keyboard.c irq.o interrupts.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)keyboard.c
	
syscall.o : $(SRC)syscall.c interrupts.o schedule.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)syscall.c

main.o : $(SRC)main.c kernel_lowlevel.o interrupts.o screen.o io.o syscall.o irq.o memory.o gdt.o tss.o schedule.o string.o keyboard.o
	$(CC) $(CFLAGS) -c -I$(INCLUDE) -o $@ $(SRC)main.c

kernel_lowlevel.o : $(SRC)kernel_lowlevel.asm
	$(NASM) -f coff -I$(INCLUDE) -o $@ $?
	
clean:
	rm *.o
		
