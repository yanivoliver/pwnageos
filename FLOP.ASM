; BIOS floppy disk reader
; Author: Roy Reznik
; Date: 4/5/2007

; read from floppy function
; "c" prototype - BOOL floppy_read(char * file_name, void * ptr_to_mem).
; 11 characters will be read from the buffer.
; maybe this function needs to be optimized by passing the args through registers.

org 100h

jmp start

%define numHeads 0002h		 ; Floppy Definition
%define sectorsPerTrack 0012h	 ; Floppy Definition
%define driveNum 00h		 ; floppy Definition

%define DIRSEG 2000h		 ; Defines where to read the directory info to.
%define CMPLEN 0Bh		 ; Defines the number of bytes to compare (FAT12 definition)
%define directoryLBA 0013h	 ; Defines the Directory LBA
%define filesPerSector 10h	 ; Number of files that enter a single sector
%define namePlaceInDirectory 00h ; The place the name is inside the Directory record
%define directoryRecordSize 20h	 ; The size of a directory record
%define clusterToSector 1Fh	 ; The number you need to add to the cluster to get the logical sector number
%define clusterNumberInRecord 1Ah ; The place where the cluster number is in the record
%define fileSizeInRecord 1Ch	 ; The place where the size of the file is in the record
%define sectorSize 0200h	 ; Sector size is 512 bytes on floppy.

filename db 'CRAP2   TXT', 00h

bits 16

start:
    mov ax, 0100h
    push ax
    mov ax, filename
    push ax

    call floppy_read
    add sp, 4

floppy_read:

    push bx
    push di
    push bp
    push dx
    push cx
    push ax
    push si
    push es

    ; move the char pointer to di
    mov bx, sp
    add bx, 12h
    mov di, [bx]

    ; move the memory pointer to bp.
    mov bx, sp
    add bx, 14h
    mov bp, [bx]

    xor cx, cx

    mov si, directoryLBA

    call calculate_chs

    xor bx, bx 			; zero out bx, which is the pointer to output
    mov ax, DIRSEG		; load the DIRSEG into es, to copy this sector into it.
    mov es, ax

read:
    mov al, 1			; read just one sector (512 bytes)
    mov ah, 2			; read function
    int 13h

    jc read_failed

    ; if we got here, read was successful.

    ; now we need to loop over all the files, and check if one of them is the file we need

    mov cx, filesPerSector		; load the number of files per sector to our loop counter
    mov bx, namePlaceInDirectory	; load the place where the name is inside a directory record

file_search_loop:
    push di
    push bx
    call strcmp				; compare strings
    add sp, 4				; clean stack
    test ax, ax				; check if strings are equal
    jz found
    add bx, directoryRecordSize
    dec cx				; decrease our count, and go with the loop!
    test cx, cx				; check if our counter is 0, else just loop again
    jnz file_search_loop

    jmp not_found

found:

    push ds					; save the value of ds
    push ax

    mov ax, es					; move es to ds, for the read bellow
    mov ds, ax

    pop ax

    mov si, [bx + clusterNumberInRecord]	; get the starting cluster from the file
    add si, clusterToSector			; convert it to sector

    mov di, [bx + fileSizeInRecord]		; taking the file size, currently just 16-bit of size.

    pop ds					; return the original value of ds

    call calculate_chs			; calculate the chs using the sector in si.

    push dx				; we need its value
    
    ; here im going to divide the size by 512 to determine how much sectors we need to read.
    mov ax, di
    mov bx, sectorSize
    xor dx, dx
    div bx

    test dx, dx				; if there is no remainder, we need to jump over the addition of 1 sector to ax.

    pop dx

    jz no_remainder

    add ax, 1

no_remainder:
    mov di, ax				; keep the number of sectors we need to read in di

    mov al, 1				; read 1 sector at a time.

    mov ah, 2				; read function   

    mov bx, bp				; move the memory output pointer to bx

read_file:

    mov ah, 2				; read function
    mov al, 1				; read 1 sector

    int 13h				; interrupt to cause read at BIOS

    jc read_file			; floppy is stuck?

    add bx, sectorSize			; advance our memory pointer

    dec di				; decrease our counter

    add si, 1				; increase the sector number

    call calculate_chs			; re-calculate the chs (cylinder, head, sector)
    
    test di, di

    jnz read_file			; if we still have sectors to read, read 'em!

    mov ax, 1				; woohoo success!

    jmp clean_all

read_failed:
    jmp read			; maybe the floppy is stuck?

not_found:

    xor ax, ax


clean_all:

    pop es
    pop si
    pop ax
    pop cx
    pop dx
    pop bp
    pop di
    pop bx

    ret

; calculate chs calculates the chs of an LBA (which is in SI when the function is called)
; this function changes cx, dx.
calculate_chs:
    push bx			; save bx so it wont change
    push ax			; save ax so it wont change

    xor dx, dx

    ; now we need to calculate the sector in si (which has the block with the directory structure)

    ; calculate the cylinder number using - Cylinder = (LBA/SectorsPerTrack)/NumHeads
    mov ax, si 			; the LBA of the directory is si
    mov bx, sectorsPerTrack 	; load the divisor to bx
    div bx
    xor dx, dx 			; zero out dx (ax now holds LBA / SectorsPerTrack)
    mov bx, numHeads 		; load the divisor to bx
    div bx 			; zero out dx (ax now holds (LBA / SectorsPerTrack) / NumHeads, which is the cylinder)

    ; now we load the cylinder to cx
    shl ax, 8
    mov cx, ax

    ; calculate the sector using Sector = (LBA mod SectorsPerTrack)+1
    xor dx, dx 			; zero out dx so it wont harm our divide
    mov ax, si 			; the LBA is si
    mov bx, sectorsPerTrack 	; load the divisor to bx
    div bx
    inc dx 			; increase dx, which now holds (LBA mod SectorsPerTrack) + 1
    and dx, 003Fh 		; be sure that only the lower 6 bits are are used
    or cx, dx 			; move the first 6 bytes from dx to cx.
    xor dx, dx

    ;calculate the head using Head = (LBA/SectorsPerTrack) mod NumHeads 
    mov ax, si 			; the LBA of the directory is si
    mov bx, sectorsPerTrack 	; load the divisor to bx
    div bx
    xor dx, dx			; zero out dx (ax now holds LBA / SectorsPerTrack)
    mov bx, numHeads		; load the divisor to bx
    div bx
    mov dh, dl			; dh will now hold the head number, I hope that the mod didn't return a value larger than 2 !!

    ; load the drive
    mov dl, driveNum

    pop ax
    pop bx

    ret

; this function is pretty simple. all it does is compare 2 strings of 11 chars, returns 0 if equal, and 1 if not
; this function's "c" prototype is something like this BOOL strcmp(char * string1, char * string2)
strcmp:
    push cx
    push di
    push si
    push bx

    ; now we need to compare the two strings using cmpsb
    mov cx, CMPLEN
    mov bx, sp
    add bx, 10
    mov di, [bx]		; load the first var from stack to di
    add bx, 2
    mov si, [bx]		; load the second var from stack to si

    cld				; compare forward

    repz cmpsb			; compare the vars

    jz success			; if everything was ok

    mov ax, 1

function_end:
    ; pop pushed registers
    pop bx
    pop si
    pop di
    pop cx

    ret

success:
    mov ax, 0
    jmp function_end

