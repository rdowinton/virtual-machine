#include "instr.h"

inline int get_opcode(int64_t instr)
{
	return (instr & 0xFF00000000000000) >> 56;
}

inline int get_addr_mode(int64_t instr, int idx)
{
	return (instr & (0xF << (idx * 4))) >> (idx * 4);
}

int get_opcode_format(enum opcode opcode)
{
	switch(opcode) {
		case HALT:
			return NO_OPERANDS;
		break;
		case PUSH:
		case POP:
		case INC:
		case DEC:
		case PRINT:
		case PRINTC:
		case GETI:
		case RAND:
		case JMP:
		case JE:
		case JNE:
		case JL:
		case JLE:
		case JG:
		case JGE:
			return ONE_OPERAND;
		case CMP:
		case MOV:
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOD:
		case XOR:
		case OR:
		case SHL:
		case SHR:
			return TWO_OPERANDS;
		default:
			return -1;
	}
}

int is_jmp(int64_t instr)
{
	enum opcode opcode = (int64_t)(instr & 0xFF00000000000000) >> 56;

	print_dbg("is_jmp instr = %016llx opcode = %d (%08x)\n", instr, opcode, opcode);
	
	return opcode == JMP | opcode == JE || opcode == JNE || opcode == JL 
			|| opcode == JLE || opcode == JG || opcode == JGE; 
}

/* size in bits */
unsigned int get_mode_size(enum addr_mode mode)
{
	switch(mode) {
		case MODE_REGISTER:
		case MODE_ADDR_REG:
			return 8;
		break;
		case MODE_CONSTANT:
		case MODE_ADDR:
			return 32;
		default:
			return -1;
	}
}

int64_t get_mode_mask(enum addr_mode mode)
{
	switch(mode) {
		case MODE_REGISTER:
		case MODE_ADDR_REG:
			return 0xFF;//return 0xF;
		break;
		case MODE_CONSTANT:
		case MODE_ADDR:
			return 0xFFFFFFFF;
		default:
			return 0x0;
	}
}


