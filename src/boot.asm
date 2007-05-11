; pwnageOS boot sector!
; Author: Yaniv Oliver
; Date: 5/4/2007

org 0h

jmp start

; 1.44Mb disks:
%define SYSSEG 140h     ; Defines the segment of the system
%define LOADERSEG (120h)    ; Our segment

bits 16
; The begining of the code
start:
    cli             ; Disable all interupts for now
    
    lidt [1200h+IDT_Pointer] ; load the idt
    lgdt [1200h+GDT_Pointer] ; load the gdt

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
