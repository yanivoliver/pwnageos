;
; pwnageOS
;
; Kernel low level functions and initializations
; to be linked with higher-level c code
;
; Author: Shimi G
;


%include "symbol.asm"

[bits 32]
; ----------
; Macros
; ----------
; Interrupt main handler
; Intel made our life harder when they made interrupts which push error codes and interrupts which doesnt push them ..
; This is why we have two diffrent basic interrupt handlers
; one which pushed a fake error code
%macro basic_interrupt_handler 1
align 16
    ;cli
    push    dword %1                ; push interrupt number
    jmp common_interrupt_handler    ; jump to common handler
%endmacro

%macro basic_interrupt_handler_with_error_code 1
align 16
    ;cli
    push    dword 0                 ; Fake error code
    push    dword %1                ; push interrupt number
    jmp common_interrupt_handler    ; jump to common handler
%endmacro

; ----------
; Defenitions
; ----------
%define KERNEL_CS               (1<<3)
%define KERNEL_DS               (2<<3)
%define NUMBER_OF_IDT_ENTRIES   (256)

; Number of bytes between the top of the stack and
; the interrupt number after the general-purpose and segment
; registers have been saved.
%define INTERRUPT_NUMBER_OFFSET (11*4)
%define INTERRUPT_ERROR_CODE_OFFSET (12*4)

; Get interrupt number from interrupt stack
%macro GET_INTERRUPT_NUMBER 1
    mov %1, [esp+INTERRUPT_NUMBER_OFFSET]
%endmacro

; Get interrupt error code from interrupt stack
%macro GET_ERROR_CODE 1
    mov %1, [esp+INTERRUPT_ERROR_CODE_OFFSET]
%endmacro   


; 8259A PIC initialization codes.
; Source: Linux bootsect.S, and Intel 8259A datasheet

; The most important reason why we reprogram the PICs is to
; route the hardware interrupts through vectors *above*
; those reserved by Intel.  The BIOS (for historical reasons :-)
; routes them such that they conflict with internal processor-generated
; interrupts.

ICW1        equ 0x11        ; ICW1 - ICW4 needed, cascade mode, interval=8,
                            ; edge triggered. (I think interval is irrelevant
                            ; for x86.)
ICW2_MASTER equ 0x20        ; put IRQs 0-7 at 0x20 (above Intel reserved ints)
ICW2_SLAVE  equ 0x28        ; put IRQs 8-15 at 0x28
ICW3_MASTER equ 0x04        ; IR2 connected to slave
ICW3_SLAVE  equ 0x02        ; slave has id 2
ICW4        equ 0x01        ; 8086 mode, no auto-EOI, non-buffered mode,
                            ; not special fully nested mode

; ----------
; Exports and Imoprts
; ----------
; Simple functions to load the IDTR, GDTR, and LDTR.
EXPORT load_idtr
EXPORT load_gdtr
EXPORT load_ldtr
EXPORT load_tr

EXPORT infinite_loop
EXPORT enter_user_mode

EXPORT configure_pic

EXPORT enable_interrupts
EXPORT disable_interrupts

EXPORT g_basic_interrupt_handlers_table
EXPORT g_basic_interrupt_handler_size
EXPORT common_interrupt_handler

EXPORT g_first_external_interrupt

IMPORT g_idt_handlers
IMPORT idle
IMPORT g_tss
IMPORT g_process
IMPORT g_tss_entry_index
IMPORT set_tss_available
IMPORT printf


; ----------
; Functions
; ----------
[SECTION .text]

; Configure PIC
; This code is taken directly from linux kernel 0.99
;
; well, that went ok, I hope. Now we have to reprogram the interrupts :-(
; we put them right after the intel-reserved hardware interrupts, at
; int 0x20-0x2F. There they won't mess up anything. Sadly IBM really
; messed this up with the original PC, and they haven't been able to
; rectify it afterwards. Thus the bios puts interrupts at 0x08-0x0f,
; which is used for the internal hardware interrupts as well. We just
; have to reprogram the 8259's, and it isn't fun.
g_first_external_interrupt  db 020h

configure_pic:
    ; Initialize master and slave PIC!
    mov al, ICW1
    out 0x20, al        ; ICW1 to master
    call    delay
    out 0xA0, al        ; ICW1 to slave
    call    delay
    mov al, ICW2_MASTER
    out 0x21, al        ; ICW2 to master
    call    delay
    mov al, ICW2_SLAVE
    out 0xA1, al        ; ICW2 to slave
    call    delay
    mov al, ICW3_MASTER
    out 0x21, al        ; ICW3 to master
    call    delay
    mov al, ICW3_SLAVE
    out 0xA1, al        ; ICW3 to slave
    call    delay
    mov al, ICW4
    out 0x21, al        ; ICW4 to master
    call    delay
    out 0xA1, al        ; ICW4 to slave
    call    delay
    mov al, 0xff        ; mask all ints in slave
    out 0xA1, al        ; OCW1 to slave
    call    delay
    mov al, 0xfb        ; mask all ints but 2 in master
    out 0x21, al        ; OCW1 to master
    call    delay
    ret
    

; Just a simple time delay
delay:
    dw   000ebh        ; jmp $+2
    ret

; Load IDTR with 6-byte pointer whose address is passed as
; the parameter.
align 8
load_idtr:
    mov eax, [esp+4]  
    lidt    [eax]
    ret

; Load the GDTR with 6-byte pointer whose address is
; passed as the parameter.  Assumes that interrupts
; are disabled.
align 8
load_gdtr:
    mov eax, [esp+4]
    lgdt    [eax]
    
    ; Reload segment registers
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp KERNEL_CS:.here
.here:
    ret

; Load the LDT whose selector is passed as a parameter.
align 8
load_ldtr:
    mov eax, [esp+4]   
    lldt    ax
    ret
    
; Load the TR whose selector is passed as a parameter.
align 8
load_tr:
    mov eax, [esp+4]   
    ltr ax
    ret
    
; Enable interrupts
enable_interrupts:
    sti
    
    mov al,0Fah       ; mask off all interrupts for now
    out 021h,al
    call delay
    
    ret

; Disable interrupts
disable_interrupts:
    cli
    ret
    
; Jump to user mode
; Deprecates
enter_user_mode:
    ; Jump to TSS
    ;mov eax, [g_tss_entry_index]
    ;shl eax, 3
    ;call set_tss_available
    ;jmp dword (5<<3):00h
    ret

; New interrupt handler
common_interrupt_handler:     
    cli

    pusha
    push ds
    push es
    push fs
    
    ; Load kernel data segment
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    
    ; Get the interrupt number
    GET_INTERRUPT_NUMBER(ebx)
    ;GET_ERROR_CODE(ebx)
    
    ; Find the appropriate interrupt handler
    mov eax, g_idt_handlers
    mov eax, [eax+ebx*4]
    
    ; Check if there is any handler
    test eax, eax
    jz .no_handler_found
    
    ; Call the found handler
    ; Add the interrupt number parameter
    mov edx, esp
    push edx
    push ebx
    call eax
    pop ebx
    pop ebx
    
    ; Continue the common handler
    jmp .continue_common_handler
    
    ; No handler found. Print appropriate message.    
.no_handler_found:
    push ebx
    push interrupt_message
    call printf
    pop eax
    pop ebx
    
.continue_common_handler :
    ; Set the tss values
    ; TODO - ESP gets wrong here I think ...
    mov eax, g_tss
    mov ebx, esp
    add ebx, 8
    mov [eax+4], ebx
    
    pop fs
    pop es
    pop ds
    popa
    
    ; Skip the interrupt number and error code
    add esp, 8  
    iretd

; Enter into an infintie loop
align 8
infinite_loop:
    nop
    jmp infinite_loop


; Interrupt handlers table
align 16
g_basic_interrupt_handlers_table:


; Make all other handlers with a macro
; Notice we already assigned interrupt 0
; Interrupts 8, 10, 11, 12, 13, 14, 17 have error code
; pushed by the processor
basic_interrupt_handler_with_error_code 0
basic_interrupt_handler_with_error_code 1
basic_interrupt_handler_with_error_code 2
basic_interrupt_handler_with_error_code 3
basic_interrupt_handler_with_error_code 4
basic_interrupt_handler_with_error_code 5
basic_interrupt_handler_with_error_code 6
basic_interrupt_handler_with_error_code 7
basic_interrupt_handler 8
basic_interrupt_handler_with_error_code 9
basic_interrupt_handler 10
basic_interrupt_handler 11
basic_interrupt_handler 12
basic_interrupt_handler 13
basic_interrupt_handler 14
basic_interrupt_handler_with_error_code 15
basic_interrupt_handler_with_error_code 16
basic_interrupt_handler 17

%assign int_number 18
%rep NUMBER_OF_IDT_ENTRIES-18
    basic_interrupt_handler_with_error_code int_number
    %assign int_number int_number+1
%endrep
interrupt_handlers_table_end:

[SECTION .data]
; Some data variables
g_basic_interrupt_handler_size      dd (((interrupt_handlers_table_end-g_basic_interrupt_handlers_table) / NUMBER_OF_IDT_ENTRIES) + 1)
interrupt_message                   db 'Interrupt 0x%X called, no deafult handler found !', 00Ah, 0
debug_message_1                     db '[%X%X]', 0

