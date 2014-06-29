#ifndef _VM_H
#define _VM_H

#include <errno.h>
#include <inttypes.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "asm_data.h"
#include "bin.h"
#include "dbg.h"
#include "instr.h"
#include "registers.h"
#include "stack.h"
#include "vm_err.h"
#include "vm_globl.h"

#define VM_VERSION	0x0003

typedef struct {
	unsigned int running;
	int registers[128];
	unsigned int stack_size;
	int *stack;
} VM_CPU;

typedef struct {
	enum addr_mode mode;
	int val;
} OPERAND;

extern VM_CPU *cpu;

#define set_ip(addr) cpu->registers[RESERVED_REG(REG_IP)] = addr

void vm_shutdown(int exit_code);

#endif

