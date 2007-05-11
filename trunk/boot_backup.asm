; pwnageOS boot sector!
; Author: Yaniv Oliver
; Date: 5/4/2007

org 0h

jmp start

; Diskette parameter table
OEMname db 'pwnageOS'
bytesPerSector dw 0200h
sectPerCluster db 01h
reservedSectors dw 0001h
numFAT db 02h
numRootDirEntries dw 00e0h
numSectors dw 0b40h
mediaType db 0f0h
numFATsectors dw 0009h
sectorsPerTrack dw 0012h
numHeads dw 0002h
numHiddenSectors dd 00000000h
numSectorsHuge dd 00000000h
driveNum db 00h
reserved db 00h
signature db 29h
volumeID dd 5a541826h
volumeLabel db 'pwnageDrive'
fileSysType db 'FAT12   '

; 1.44Mb disks:
%define SYSSEG 100h     ; Defines the segment of the system
%define LOADERSEG (7C0h)    ; Our segment

bits 16
; The begining of the code
start:
    cli         ; Interupts off
    xor ax, ax
    mov ss, ax
    mov sp, 7C00h       ; 7C00h - The address our code is loaded to.
    push ss         ; Set es to 0 (value of ss)
    pop es

    mov bx, 0078h       ; The INT 1E vector is at 0078
    lds si, [ss:bx]     ; Set DS:SI to 0:78

    ; Move the diskette param table to 0:7c3e (where is should be on the diskette)
    mov di, 7C3Eh       ; Set the position to copy the table
    mov cx, 0Bh     ; Set the amount of bytes to copy
    cld         ; Clear the copy direction (copy forward)
    repz movsb      ; Copy a byte each time until cx == 0

    push es         ; Set ds to 0 (value of es)
    pop ds

    ; Alter the new copy of the param table data
    mov byte [di-2h], 0Fh   ; Change the head settle time (0:7c47)
    mov cx, [sectorsPerTrack]; Sectors per track 
    mov byte [di-7h], cl    ; Set the sectors per track

    ; Change INT 1E so it points to the new table
    mov [bx+2h], ax     ; Change INT 1E's segment
    mov word [bx], 7C3Eh    ; Change the offset of the table

    sti         ; Turn interupts back on
    int 13          ; Call int 13 with ax = 0 in order to use the new table
    jb int13_error      ; Check if the call worked, if it didn't we cant continue.

    mov si, (7C00h+startup_msg) ; Print the startup string
    call print_string

    ; Turn on A20 handling (20 bit memory access)
    mov dx, 92h     ; We output to port 92h
    mov al, 02h     ; We output the value 02h, turn on A20 handling
    out dx, al      ; DO IT
    
    mov ax, SYSSEG
    mov es, ax      ; Set the position we'll start copying into, SYSSEG:0
    mov al, 011h      ; We want to read 2 sectors
    call read_sectors

    cli             ; Disable all interupts for now
    
    lidt [7C00h+IDT_Pointer] ; load the idt
    lgdt [7C00h+GDT_Pointer] ; load the gdt

    ; Jump to protected mode
    mov ax, 1h          ; protected mode (PE) bit
    lmsw    ax          ; This is it!

    ; Jump to first gdt descriptor which is CS
    ; its a far jump to CS segment and bootloader_32_bit address
    ; we will get here into 32 bit mode
    jmp dword (1<<3) : ((LOADERSEG << 4) + bootloader_32_bit)

bits 32
bootloader_32_bit:
    ; Set up the data segment to be 32 bit compatible
    mov ax, (2<<3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set the stack
    mov esp, (4 * 1024) ; 4k of stack :P
    
    ; Now we are in 32-bit protected mode
    ; we will jump here to the native c kernel code    
    jmp (1<<3) : (SYSSEG << 4)
    
bits 16
int13_error:
    mov si, (7C00h+int13error_msg) ; Print the int13 error string
    call print_string
    jmp fin

; Halt loop
fin:
    hlt
    jmp fin

; This function reads the data to the segment passed in es, begining at es:0h
; Also, al should be the amount of sectors to read.
read_sectors:
    mov byte [sectors_to_read], al  ; Save the amount of sectors we need to read
    mov byte [sectors_read], 00h    ; Set the amount of sectors read to 0
    mov bx, 0h

read:
    mov ah, 02h         ; Read from diskette function
    mov al, 01h         ; Number of segments to read
    mov ch, 00h         ; Cylinder number
    mov cl, [sectors_read]  ; Segment to read, copy the amount of sectors read
    add cl, 2h          ; Read 2 sectors in front of the amount we read, as we start reading from sector 2h (after MBR)
    mov dh, 00h         ; Read from first head
    mov dl, 00h         ; Drive, read from first drive (A:)

    int 13h         ; READ!
    jc read_failed      ; If the read failed, jmp

    inc byte [sectors_read] ; If we succeeded reading, increment the amount of sectors read.
    add bx, 200h        ; Start reading next sector to 512 bytes forward (each sector is 512 bytes)

    mov al, [sectors_read]  ; Copy the amount of sectors read
    cmp [sectors_to_read], al   ; Compare the amount of sectors we need to read to the amount we read.
    jnz read            ; If we still have more to read, jmp back and read more

    ret             ; If we got here we're done, YEY

read_failed:
    jmp read        ; Try to read the current sector again (floppy sucks, may fail some reads)

; A procedure that prints a string. The string should be point to by si (real mode)
print_string:
    lodsb           ; Get a char from ds:si
    or al, al       ; Check if the char is a 0
    jz return       ; If it is, we're done.

    mov ah, 0Eh     ; Write one character
    mov bx, 0007h       ; Write to page 0, set the background
    int 10h         ; Call the interupt

    jmp print_string

return:
    ret

; VARIABLES
sectors_read db 00h ; Holds the amount of sectors read
sectors_to_read db 00h  ; holds the amount of sectors to read

; STRINGS
startup_msg             db "Starting up pwnageOS!", 0dh, 0ah, 0h
int13error_msg          db "Error setting int13", 0dh, 0ah, 0h

align 8, db 0
GDT:
    ; Descriptor 0 is not used
    dw 0
    dw 0
    dw 0
    dw 0

    ; Descriptor 1: kernel code segment
    dw 0xFFFF   ; bytes 0 and 1 of segment size
    dw 0x0000   ; bytes 0 and 1 of segment base address
    db 0x00     ; byte 2 of segment base address
    db 0x9A     ; present, DPL=0, non-system, code, non-conforming,
            ;   readable, not accessed
    db 0xCF     ; granularity=page, 32 bit code, upper nibble of size
    db 0x00     ; byte 3 of segment base address

    ; Descriptor 2: kernel data and stack segment
    ; NOTE: what Intel calls an "expand-up" segment
    ; actually means that the stack will grow DOWN,
    ; towards lower memory.  So, we can use this descriptor
    ; for both data and stack references.
    dw 0xFFFF   ; bytes 0 and 1 of segment size
    dw 0x0000   ; bytes 0 and 1 of segment base address
    db 0x00     ; byte 2 of segment base address
    db 0x92     ; present, DPL=0, non-system, data, expand-up,
            ;   writable, not accessed
    db 0xCF     ; granularity=page, big, upper nibble of size
    db 0x00     ; byte 3 of segment base address

GDT_Pointer:
    dw 3*8              ; limit (amount of gdt entries * their size)
    dd (LOADERSEG<<4) + GDT     ; base address

IDT_Pointer:
    dw 0
    dd 00
