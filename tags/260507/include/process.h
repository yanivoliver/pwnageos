/*
Syscall handler
Author: Shimi G
*/

#ifndef HEADER_PWNAGE_PROCESS
#define HEADER_PWNAGE_PROCESS

typedef struct process_rec process_t;

#include "syscall.h"
#include "keyboard.h"
#include "screen.h"

/* Process*/

struct process_rec {
	ulong_t process_id;
	registers_t registers;
	bool_t blocking;
	syscall_entry_t * blocking_syscall;
	input_t input;
	uchar_t name[STRING_BUFFER];
	console_t console;
	struct process_rec * next_process;
};

#endif