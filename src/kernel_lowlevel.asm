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
%macro basic_trap_handler 1
align 16
	;cli
    push    dword %1                ; push interrupt number
    jmp common_trap_handler    ; jump to common handler
%endmacro

%macro basic_trap_handler_with_error_code 1
align 16
	;cli
    ;push    dword 0                 ; Fake error code
    push    dword %1                ; push interrupt number
    jmp common_trap_handler_push_error_code    ; jump to common handler
%endmacro

%macro basic_irq_handler_with_error_code 1
align 16
    push    dword 0                 ; Fake error code
    push    dword %1                ; push interrupt number
    jmp common_irq_handler    ; jump to common irq handler
%endmacro

; ----------
; Defenitions
; ----------
%define KERNEL_CS						(1<<3)
%define KERNEL_DS						(2<<3)
%define USER_CS							((3<<3)|(0x0003))
%define USER_DS							((4<<3)|(0x0003))
%define NUMBER_OF_IDT_ENTRIES			(256)

%define STACK_DONT_FIX					(0)
%define STACK_FIX_KERNEL_TO_USER		(1)
%define STACK_FIX_USER_TO_KERNEL		(2)

%define FIRST_EXTERNAL_INTERRUPT		(030h)
%define NUMBER_OF_EXTERNAL_INTERRUPTS	(0Fh)

; Number of bytes between the top of the stack and
; the interrupt number after the general-purpose and segment
; registers have been saved.
%define INTERRUPT_NUMBER_OFFSET (11*4)
%define INTERRUPT_ERROR_CODE_OFFSET (12*4)

%define LATCH					(1193180/500)

; Get interrupt number from interrupt stack
%macro GET_INTERRUPT_NUMBER 1
    mov %1, [esp+INTERRUPT_NUMBER_OFFSET]
%endmacro

; Get interrupt error code from interrupt stack
%macro GET_ERROR_CODE 1
    mov %1, [esp+INTERRUPT_ERROR_CODE_OFFSET]
%endmacro

; Get the CS
%macro GET_CS 1
    mov %1, [esp+(14*4)]
%endmacro

; Get the EFLAGS
%macro GET_EFLAGS 1
    mov %1, [esp+(15*4)]
%endmacro

; Set the EFLAGS
%macro SET_EFLAGS 1
    mov dword [esp+(15*4)], %1
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
ICW2_MASTER equ 0x30        ; put IRQs 0-7 at 0x30 (above Intel reserved ints)
ICW2_SLAVE  equ 0x38        ; put IRQs 8-15 at 0x38
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
EXPORT get_eflags

EXPORT g_basic_interrupt_handlers_table
EXPORT g_basic_interrupt_handler_size
EXPORT common_interrupt_handler

EXPORT g_first_external_interrupt

EXPORT getchar
EXPORT getch
EXPORT putch
EXPORT gets
EXPORT puts
EXPORT fread

EXPORT scheduler_low

IMPORT irq_send_ack
IMPORT g_irq_handlers
IMPORT g_idt_handlers
IMPORT idle
IMPORT g_tss
IMPORT g_process
IMPORT g_tss_entry_index
IMPORT set_tss_available
IMPORT printf
IMPORT g_scheduler_fixed_stack


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
g_first_external_interrupt  db FIRST_EXTERNAL_INTERRUPT

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
    
    ; Configure timer
	mov al, 036h
	out 043h, al
	mov al, LATCH & 0ffh
	out 040h, al
	mov al, LATCH >> 8
	out 040h, al
    
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
    
common_irq_handler:
	pusha
	push ds
	push es
	push fs

	; Load kernel data segment
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    
    ; Calculate the irq number
    GET_INTERRUPT_NUMBER(ebx)
    sub ebx, FIRST_EXTERNAL_INTERRUPT
    dec ebx
    
    ; Call irq handler
    mov eax, g_irq_handlers
    mov eax, [eax+ebx*4]
    
    ; Check if there is any handler
    test eax, eax
    jz .irq_no_handler_found
    
    mov edx, esp
    push edx
    push ebx
    ;call eax
    add esp, 08h
    
    ; No handler found. Print appropriate message.    
.irq_no_handler_found:
    xor ecx, ecx
    push debug_message_1
    push ecx
    call printf
    add esp, 08h

.continue_common_irq_handler:
    ; Send ack to the PIC
    push ebx
    call irq_send_ack
    add esp, 04h

    ; Check for DPC
    ; Call DPC task
    
    pop fs
    pop es
    pop ds
    popa
    
    ; Skip the interrupt number and error code
    add esp, 8
    
    iretd

common_trap_handler_push_error_code:
	push eax
	push eax
	mov eax, [esp+08h]
	mov dword [esp+08h], 0
	mov dword [esp+04h], eax
	pop eax
	jmp common_trap_handler

common_trap_handler:
	pusha
	push ds
	push es
	push fs

	; Load kernel data segment
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    
    GET_INTERRUPT_NUMBER(ebx)
    GET_ERROR_CODE(eax)
    
    xor ecx, ecx
    push eax
    push ebx
    push interrupt_message_error
    push ecx
    call printf
    add esp, 010h

.continue_common_trap_handler:   
    pop fs
    pop es
    pop ds
    popa
    
    ; Skip the interrupt number and error code
    add esp, 8
    
    iretd
    
; Low schduler
scheduler_low:
	iretd
	

; New interrupt handler
common_interrupt_handler:
    pusha
    push ds
    push es
    push fs
    
    ; Load kernel data segment
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    
    ; Update the IF to be set, KERNEL and USER
    GET_EFLAGS(ecx)
    or ecx, 0x200
    SET_EFLAGS(ecx)
    
    ; Get the interrupt number
    GET_INTERRUPT_NUMBER(ebx)
    ;GET_ERROR_CODE(ebx)
        
    ; Set the TSS values (which could be overwritten by interrupt handlers)
    mov ecx, g_tss 
    mov edx, esp
    add edx, 048h
    mov [ecx+4], edx
    
    ; Find the appropriate interrupt handler
    mov eax, g_idt_handlers
    mov eax, [eax+ebx*4]
    
    ; Check if there is any handler
    test eax, eax
    jz .no_handler_found
    
    ; Check if its a critical interrupt which cannot be interrupted
    ;;cmp ebx, FIRST_EXTERNAL_INTERRUPT	; Which is the scheduler interrupt
    ;;jnz .interrupt_not_critical
    cmp ebx, FIRST_EXTERNAL_INTERRUPT
    jl .interrupt_not_critical
    cmp ebx, FIRST_EXTERNAL_INTERRUPT+NUMBER_OF_EXTERNAL_INTERRUPTS
    jg .interrupt_not_critical
    
    ; ** Critical interrupt    
    ; Call the found handler
    ; Add the interrupt number parameter
    mov edx, esp
    push edx
    push ebx
    call eax
    
    ; We should pop of this values only if the stack wasnt fixed by the scheduler handler
    lea eax, [g_scheduler_fixed_stack]
    cmp dword [eax], STACK_DONT_FIX
    jz .interrupt_pop_handler_arguments
    
    cmp dword [eax], STACK_FIX_KERNEL_TO_USER
    jnz .interrupt_check_user_fix
    
    mov dword [eax], STACK_DONT_FIX
    jmp .interrupt_post_handler
    
.interrupt_check_user_fix:
	cmp dword [eax], STACK_FIX_USER_TO_KERNEL
    jnz .interrupt_post_handler
    add esp, 010h
    mov dword [eax], STACK_DONT_FIX
    jmp .continue_internal_kernel_handler

.interrupt_pop_handler_arguments:
    pop ebx
    pop ebx

.interrupt_post_handler:
    ; Continue the common handler
    jmp .continue_common_handler
    
    ; ** Non critical interrupt
.interrupt_not_critical:
	sti
	mov edx, esp
    push edx
    push ebx
    call eax
    pop ebx
    pop ebx
	cli
    
    ; Continue the common handler
    jmp .continue_common_handler
    
    ; No handler found. Print appropriate message.    
.no_handler_found:
	xor eax, eax
    push ebx
    push interrupt_message
    push eax
    call printf
    pop eax
    pop eax
    pop ebx
    
.continue_common_handler:   
    pop fs
    pop es
    pop ds
    popa
    
    ; Skip the interrupt number and error code
    add esp, 8
    
    iretd
    
.continue_internal_kernel_handler: 
	; When switching two threads inside the kernel,
	; we will use simple ret. So we need to fix the stack alittle ..
	; Backup for EAX will be written on the new thread's stack
	; Then EAX will be filled with the new esp
	; then we will write on the new thread's stack the return address and eflags
	; then we will set the new ESP, pop the original value of eax (from the second stack)
	; and do a ret (which will pop off EIP from the second stack)
	; We assume that thread switched doesnt use another CS...
	;  ## BEFORE
	;  | Thread 1 Stack |     | Thread 2 Stack |
	;  | EIP            |	  |				   |
	;  | CS	  	    	|	  |				   |
	;  | EFLAGS 		|     |				   |
	;   
	;  ## AFTER
	;  | Thread 1 Stack |     | Thread 2 Stack |
	;  |				|	  |	ORIGINAL EAX   |
	;  |				|     | EFLAGS		   |
	;  |				|	  |	EIP			   |
	pop fs
	pop es
	pop ds
	mov eax, dword [esp+0Ch]		; Load the new ESP
	mov ebx, dword [esp+028h]		; Load the new EIP
	mov ecx, dword [esp+02Ch]		; Load the new CS
	mov edx, dword [esp+030h]		; Load the new EFLAGS
	mov dword [eax-04h], edx		; Write to the destination stack the EFLAGS
	mov dword [eax-08h], ecx		; Write to the destination stack the CS
	mov dword [eax-0Ch], ebx		; Write to the destination stack the EIP
	
	; Put the new ESP on the stack
	push eax
	add esp, 04h
	
	; Pop all registers
	popa
	
	; Clear the stack
	add esp, 014h
	
	; Load the new esp which is lower on the stack
	xchg esp, dword [esp-038h]
	
	; We loaded 3 arguments to the stack
	sub esp, 0Ch
	
	; Just make a regular iret
	iret

; Get eflags
get_eflags:
	pushf
	pop eax
	ret

; Enter into an infintie loop
align 8
infinite_loop:
    nop
    jmp infinite_loop
    
; int getch();
getch:
    push ebp
    mov ebp, esp
    push ebx
    
    mov ah, 008h
    int 021h
    and eax, 0000000FFh
    
    ;lea ebx, [ebp+8]
    ;mov dword [ebx], 000000000h
    ;mov byte [ebx], al
    
    pop ebx                  
    pop ebp
    ret
    
; int getchar();
getchar:
    push ebp
    mov ebp, esp
    push ebx
    
    mov ah, 001h
    int 021h
    and eax, 0000000FFh
    
    ;lea ebx, [ebp+8]
    ;mov dword [ebx], 000000000h
    ;mov byte [ebx], al
    
    pop ebx                  
    pop ebp
    ret
    
; int gets(char * buffer);
gets:
    push ebp
    mov ebp, esp
    push ebx
    push edx
    
    mov ah, 00Ah
    mov edx, [ebp+8]
    int 021h
    
    pop edx
    pop ebx                  
    pop ebp
    ret
    
; int fread(void * buffer, ulong_t length);
fread:
    push ebp
    mov ebp, esp
    push ebx
    push edx
    
    mov ah, 0F0h
    mov ebx, [ebp+0Ch]
    mov edx, [ebp+08h]
    int 021h
    
    pop edx
    pop ebx                  
    pop ebp
    ret
    
; void puts(char * buffer);
puts:
    push ebp
    mov ebp, esp
    push ebx
    push edx
    
    mov ah, 009h
    mov edx, [ebp+8]
    int 021h
    
    pop edx
    pop ebx                  
    pop ebp
    ret
    
 ; int putch(int c);
putch:
    push ebp
    mov ebp, esp
    push edx
    
    mov ah, 002h
    mov dl, byte [ebp+8]
    int 021h
    and eax, 0000000FFh
    
    pop edx
    pop ebp
    ret


; Interrupt handlers table
align 16
g_basic_interrupt_handlers_table:


; Make all other handlers with a macro
; Notice we already assigned interrupt 0
; Interrupts 8, 10, 11, 12, 13, 14, 17 have error code
; pushed by the processor
basic_trap_handler_with_error_code 0
basic_trap_handler_with_error_code 1
basic_trap_handler_with_error_code 2
basic_trap_handler_with_error_code 3
basic_trap_handler_with_error_code 4
basic_trap_handler_with_error_code 5
basic_trap_handler_with_error_code 6
basic_trap_handler_with_error_code 7
basic_trap_handler 8
basic_trap_handler_with_error_code 9
basic_trap_handler 10
basic_trap_handler 11
basic_trap_handler 12
basic_trap_handler 13
basic_trap_handler 14
basic_trap_handler_with_error_code 15
basic_trap_handler_with_error_code 16
basic_trap_handler 17

%assign int_number 18
%rep 47-18
    basic_trap_handler_with_error_code int_number
    %assign int_number int_number+1
%endrep

%assign int_number 030h
%rep 0Fh
    basic_irq_handler_with_error_code int_number
    %assign int_number int_number+1
%endrep

%assign int_number 64
%rep NUMBER_OF_IDT_ENTRIES-64
    basic_trap_handler_with_error_code int_number
    %assign int_number int_number+1
%endrep
interrupt_handlers_table_end:

[SECTION .data]
; Some data variables
g_basic_interrupt_handler_size      dd (((interrupt_handlers_table_end-g_basic_interrupt_handlers_table) / NUMBER_OF_IDT_ENTRIES) + 1)
interrupt_message                   db 'Interrupt 0x%X called, no deafult handler found !', 00Ah, 0
interrupt_message_error             db '[0x%X, #%X]', 00Ah, 0
debug_message_1                     db '[%X%X]', 0

