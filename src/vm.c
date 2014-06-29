/*
 * Author: Richard Dowinton
 * 
 * Virtual machine which interprets bytecode
 *
 * Instruction size: 64 bit
 *
 * Mode = 4 bit (each param's mode is located from lsb, so first param offset is lsb, second is lsb + 4 and so on)
 * Opcodes = 8 bit
 * Registers = 8 bit
 * Addresses = 32 bit
 * Constants = 32 bit
 *
 */

#include "vm.h"

VM_CPU *cpu;

void vm_shutdown(int exit_code)
{
	exit(exit_code);
}

int get_val(OPERAND *operand)
{
	switch(operand->mode) {
		case MODE_CONSTANT:
			return operand->val;
		break;
		case MODE_REGISTER:
			return cpu->registers[operand->val];
		break;
		case MODE_ADDR:
			return (int)&data_section[operand->val];
		break;
		default:
			return -1;
	}
}

void jmp_op(enum opcode opcode, OPERAND **operands)
{
	int addr = get_val(operands[0]) - code_offset;
	int cmp = cpu->registers[RESERVED_REG(REG_CMP)];

	print_dbg("jmp_op(): addr = %d code_offset = %d\n", addr, code_offset);

	addr -= sizeof(int64_t); // Account for += sizeof(int64_t) after eval() in cycle()
	
	switch(opcode) {
		case JMP:
			set_ip(addr);
		break;
		case JE:
			if(cmp == 0) set_ip(addr);
		break;
		case JNE:
			if(cmp != 0) set_ip(addr);
		break;
		case JL:
			if(cmp == 2) set_ip(addr);
		break;
		case JLE:
			if(cmp == 0 || cmp == 2) set_ip(addr);
		break;
		case JG:
			if(cmp == 1) set_ip(addr);
		break;
		case JGE:
			if(cmp == 0 || cmp == 1) set_ip(addr);
		break;
	}
}

void math_op(enum opcode opcode, OPERAND **operands)
{
	int vals[2];

	vals[0] = get_val(operands[0]);
	vals[1] = get_val(operands[1]);

	switch(opcode) {
		case ADD:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] + vals[1];
		break;
		case SUB:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] - vals[1];
		break;
		case MUL:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] * vals[1];
		break;
		case DIV:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] / vals[1];
		break;
		case MOD:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] % vals[1];
		break;
		case XOR:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] ^ vals[1];
		break;
		case OR:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] | vals[1];
		break;
		case SHL:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[0] << vals[1];
		break;
		case SHR:
			cpu->registers[RESERVED_REG(REG_MATH)] = vals[1] >> vals[0];
		break;
	}
}

/*
 * Addr mode must be used for strings 
 */
void eval(int64_t instr)
{
	enum opcode opcode = GET_OPCODE(instr);
	enum opcode_format format = get_opcode_format(opcode);
	OPERAND *operands[2];
	int vals[2];

	print_dbg("eval(): opcode: %d instr: 0x%016llx (ip: %d)\n", opcode, instr, cpu->registers[RESERVED_REG(REG_IP)]);

	switch(format) {
		case NO_OPERANDS:
		break;
		case ONE_OPERAND:
			operands[0] = malloc(sizeof(OPERAND));
			operands[0]->mode = GET_ADDR_MODE(instr, 0);
			operands[0]->val = (instr >> (56 - get_mode_size(operands[0]->mode))) & get_mode_mask(operands[0]->mode);

			print_dbg("Operand one: mode = %d, value = %d\n", operands[0]->mode, operands[0]->val);
		break;
		case TWO_OPERANDS:
			operands[0] = malloc(sizeof(OPERAND));
			operands[0]->mode = GET_ADDR_MODE(instr, 0);
			operands[0]->val = (instr >> (56 - get_mode_size(operands[0]->mode))) & get_mode_mask(operands[0]->mode); 

			operands[1] = malloc(sizeof(OPERAND));
			operands[1]->mode = GET_ADDR_MODE(instr, 1);
			operands[1]->val = (instr >> (56 - get_mode_size(operands[0]->mode) - get_mode_size(operands[1]->mode)))
						& get_mode_mask(operands[1]->mode);
	
			print_dbg("Operand one: mode = %d, value = %d\n", operands[0]->mode, operands[0]->val);
			print_dbg("Operand two: mode = %d, value = %d\n", operands[1]->mode, operands[1]->val);
		break;
	}

	switch(opcode) {
		case HALT:
			cpu->running = 0;
		break;
		case MOV:
			if(operands[1]->mode != MODE_REGISTER) {
				vm_error(VM_ERR_FATAL, "mov: destination is not a register\n");
			}

			cpu->registers[operands[1]->val] = get_val(operands[0]);
		break;
		case PUSH:
			push(get_val(operands[0]));
		break;
		case POP:
			if(operands[0]->mode == MODE_REGISTER) {
				cpu->registers[operands[0]->val] = pop();
			}
		break;
		case INC:
			if(operands[0]->mode != MODE_REGISTER) {
				vm_error(VM_ERR_FATAL, "inc: operand is not a register\n");
			}

			cpu->registers[operands[0]->val]++;
		break;
		case DEC:
			if(operands[0]->mode != MODE_REGISTER) {
				vm_error(VM_ERR_FATAL, "dec: operand is not a register\n");
			}

			cpu->registers[operands[0]->val]--;
		break;
		case PRINT:
			if(operands[0]->mode == MODE_CONSTANT) {
				printf("%d", operands[0]->val);
				print_dbg("PRINT (MODE_CONSTANT): [%d]\n", operands[0]->val);
			} else if(operands[0]->mode == MODE_REGISTER) {
				printf("%d", cpu->registers[operands[0]->val]);
				print_dbg("PRINT (MODE_REGISTER): [%d]\n", operands[0]->val);
			} else if(operands[0]->mode == MODE_ADDR) {
				printf("%s", &data_section[operands[0]->val]);
				print_dbg("PRINT (MODE_ADDR): [%d][%s]\n", operands[0]->val, &data_section[operands[0]->val]);
			}
		break;
		case PRINTC:
			printf("%c", get_val(operands[0]));
		break;
		case GETI:
			if(operands[0]->mode != MODE_REGISTER) {
				vm_error(VM_ERR_FATAL, "geti: destination is not a register\n");
			}

			scanf("%d", &cpu->registers[operands[0]->val]);
		break;
		case RAND:
			if(operands[0]->mode != MODE_REGISTER) {
				vm_error(VM_ERR_FATAL, "rand: destination is not a register\n");
			}

			cpu->registers[operands[0]->val] = rand() % 255;
		break;
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOD:
		case XOR:
		case OR:
		case SHL:
		case SHR:
			math_op(opcode, operands);
		break;
		case CMP:
			vals[0] = get_val(operands[0]);
			vals[1] = get_val(operands[1]);

			if(vals[0] == vals[1]) {
				cpu->registers[RESERVED_REG(REG_CMP)] = 0;
			} else if(vals[0] > vals[1]) {
				cpu->registers[RESERVED_REG(REG_CMP)] = 1;
			} else if(vals[0] < vals[1]) {
				cpu->registers[RESERVED_REG(REG_CMP)] = 2;
			}
		break;
		case JMP:
		case JE:
		case JNE:
		case JL:
		case JLE:
		case JG:
		case JGE:
			jmp_op(opcode, operands);
		break;
	}
}

void cycle()
{
	while(cpu->running) {
		//eval(*(int64_t*)&code_section[cpu->registers[RESERVED_REG(REG_IP)] += sizeof(int64_t)]);
		eval(*(int64_t*)&code_section[cpu->registers[RESERVED_REG(REG_IP)]]);
		cpu->registers[RESERVED_REG(REG_IP)] += sizeof(int64_t);
	}
}

void print_data()
{
	int i;

	print_dbg("Data offset: %d size: %d\n", data_shdr->offset, data_shdr->size);

	for(i = 0; i < data_shdr->size; i++) {
		print_dbg("data[%d]: 0x%02x %c\n", i, data_section[i], data_section[i]);
	}
}

void print_code()
{
	int i;

	print_dbg("Code offset: %d size: %d\n", code_shdr->offset, code_shdr->size);

	for(i = 0; i < code_shdr->size; i += sizeof(int64_t)) {
		print_dbg("code[%d]: 0x%016llx\n", i, *(int64_t*)&code_section[i]); 
	}
}

test()
{
	int64_t test = 0x1015000003e80010LL; 
	eval(test);
}

int main(int argc, char **argv)
{
	int c;

	while((c = getopt(argc, argv, "s:")) != -1) {
		switch(c) {
			case 's':

			break;
			default:
				abort();
		}
	}

	if(optind >= argc) {
		fprintf(stderr, "Usage: %s [opts] <bin>\n", argv[0]);
		return 1;
	}

	srand(time(NULL));

	load_bin(argv[optind]);

	print_data();
	print_code();

	cpu = malloc(sizeof(VM_CPU));
	
	int i;

	for(i = optind + 1; i < argc; i++) {
		cpu->registers[ARG_REG(i)] = (int)argv[i];	
	}
	
	cpu->stack_size = bin_hdr->stack_size;
	cpu->stack = malloc(sizeof(int) * cpu->stack_size);
	cpu->registers[RESERVED_REG(REG_STACKP)] = cpu->stack_size;
	
	cpu->registers[RESERVED_REG(REG_IP)] = bin_hdr->entry_point;
	print_dbg("Set IP: %d\n", cpu->registers[RESERVED_REG(REG_IP)]);
	cpu->running = 1;
	cycle();
}


