#ifndef _INSTR_H
#define _INSTR_H

#include <stdint.h>

#define INSTR_HALT      0
#define INSTR_LDC       1
#define INSTR_CPR		12
#define INSTR_PUSH      2
#define INSTR_POP       3
#define INSTR_PUSHC     4
#define INSTR_ADD       5
#define INSTR_SUB       6
#define INSTR_MUL       7
#define INSTR_DIV       8
#define INSTR_MOD       9
#define INSTR_XOR       10
#define INSTR_PRINTR    11

#define GET_OPCODE(instr) ((instr & 0xFF00000000000000) >> 56)
#define GET_ADDR_MODE(instr, idx) ((instr & (0xF << (idx * 4))) >> (idx * 4))

enum opcode {
	HALT	= 0,
	LDC 	= 1,
	CPR		= 12,
	PUSH	= 2,
	POP		= 3,
	PUSHC	= 4,
	ADD		= 5,
	SUB		= 6,
	MUL		= 7,
	DIV		= 8,
	MOD		= 9,
	XOR		= 10,
	PRINT	= 11,
	OR		= 13,
	SHL		= 14,
	SHR		= 15,
	CMP		= 16,
	JMP		= 17,
	JE		= 18,
	JNE		= 19,
	JL		= 20,
	JLE		= 21,
	JG		= 22,
	JGE		= 23,
	MOV		= 24,
	GETI	= 25,
	RAND	= 26,
	PRINTC	= 27,
	INC		= 28,
	DEC		= 29
};

enum opcode_format {
	NO_OPERANDS		= 0,
	ONE_OPERAND		= 1,
	TWO_OPERANDS	= 2
};

enum addr_mode {
	MODE_REGISTER 	= 0,
	MODE_CONSTANT 	= 1,
	MODE_ADDR		= 2,
	MODE_ADDR_REG	= 3 
};

int get_opcode_format(enum opcode opcode);
int is_jmp(int64_t instr);
unsigned int get_mode_size(enum addr_mode mode);
int64_t get_mode_mask(enum addr_mode mode);

#endif

