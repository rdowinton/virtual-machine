#include "asm_code.h"

unsigned int us_count;
char **unk_subs;

unsigned int ua_count;
char **unk_addrs;

int get_var_addr(BIN_DATA *bin, char *name)
{
	int i;

	print_dbg("get_var_addr(): %s\n", name);

	for(i = 0; i < bin->num_data; i++) {
		print_dbg("Search: %s\n", bin->data_names[i]);

		if(strcmp(bin->data_names[i], name) == 0) {
			print_dbg("Found: %d\n", bin->data_entries[i]->offset);

			return bin->data_entries[i]->offset;
		}
	}

	return -1;
}

void fix_var_addrs(BIN_DATA *bin, char *buf, unsigned int *addr_refs, unsigned int aref_c)
{
	int64_t *instr;
	int addr;

	while(aref_c--) {
		instr = (int64_t*)&buf[addr_refs[aref_c]]; 

		switch(get_opcode_format(GET_OPCODE(*instr))) {
			case ONE_OPERAND:
				addr = get_var_addr(bin, unk_addrs[(*instr & 0x00FFFFFFFF) >> 24]);
				//print_dbg("before instr: 0x%016llx\n", *instr);
				*instr &= 0xFF0000000000000F; // Clear operands
				*instr |= addr << 24;
				//print_dbg("addr: 0x%016llx\n", ((int64_t)addr << 24));
				print_dbg("fix_var_addrs() ONE_OPERAND addr = %d (0x%016llx), instr = 0x%016llx\n", addr, (int64_t)addr, *instr);
			break;
			case TWO_OPERANDS:
				if(GET_ADDR_MODE(*instr, 0) == MODE_ADDR) {
					addr = get_var_addr(bin, unk_addrs[(*instr & 0x00FFFFFFFF) >> 24]);
					*instr |= (int64_t)addr << 24;	
					print_dbg("fix_var_addrs() TWO_OPERAND 0 addr = %d, instr = 0x%016llx\n", addr, instr);
				}
				
				// TODO: Need to check size of first operand
				if(GET_ADDR_MODE(*instr, 1) == MODE_ADDR) {
					addr = get_var_addr(bin, unk_addrs[(*instr & 0x0000FFFFFFFF) >> 16]);
					*instr |= (int64_t)addr << 16;
					print_dbg("fix_var_addrs() TWO_OPERAND 1 addr = %d, instr = 0x%016llx\n", addr, *instr);
				}
			break;
		}
	}
}

int get_sub_addr(BIN_DATA *bin, char *subname)
{
	SUBROUTINE_HDR *sub;
	int i;

	print_dbg("Get sub addr: %s\n", subname);
	print_dbg("num subs: %d\n", bin->num_subs);

	for(i = 0; i < bin->num_subs; i++) {
		sub = bin->subs[i];

		print_dbg("sub[%d]->name = [%s]\n", i, sub->name);

		if(strcmp(sub->name, subname) == 0) {
			print_dbg("Found sub: %d\n", (int)sub->offset);

			return (int)sub->offset;
		}
	}

	return -1;
}

void fix_jmps(BIN_DATA *bin, char *buf, unsigned int *jmps, int jmpc)
{
	int64_t *instr;
	enum opcode opcode;
	unsigned int addr;
	
	while(jmpc--) {
		 instr = (int64_t*)&buf[jmps[jmpc]];
		 print_dbg("Fix jmp instr: %016llx (offset %d) (jmpc = %d)\n", *instr, jmps[jmpc], jmpc);
		 print_dbg("Unknown index: %d\n", ((*instr & 0x00FFFFFFFF) >> 24));
		 addr = get_sub_addr(bin, unk_subs[(*instr & 0x00FFFFFFFF) >> 24]);
		 *instr &= 0xFF0000000000000F;
		 *instr |= (int64_t)addr << 24;
		 print_dbg("Fixed jmp instr: %016llx\n", *instr);
	}
}

int parse_int(char *token)
{
	if(strncmp(token, "0x", 2) == 0) {
		return strtol(token, NULL, 0);
	} else {
		return atoi(token);
	}
}

int parse_reg(char *token)
{
	if(strncmp(token, "ra", 2) == 0) {
		return REG_RES_OFFSET + REG_RADDR;
	} else if(strncmp(token, "rv", 2) == 0) {
		return REG_RES_OFFSET + REG_RVAL;
	} else if(strncmp(token, "ip", 2) == 0) {
		return REG_RES_OFFSET + REG_IP;
	} else if(strncmp(token, "bp", 2) == 0) {
		return REG_RES_OFFSET + REG_BASEP;
	} else if(strncmp(token, "sp", 2) == 0) {
		return REG_RES_OFFSET + REG_STACKP;
	} else if(strncmp(token, "cmp", 3) == 0) {
		return REG_RES_OFFSET + REG_CMP;
	} else if(strncmp(token, "out", 3) == 0) {
		return REG_RES_OFFSET + REG_OUTPUT;
	} else if(*token >= '0' && *token <= '9') {
		return parse_int(token);
	} else {
		switch(*token) {
			case 'm':
				return RESERVED_REG(REG_MATH);
			break;
			case 'a':
				return REG_ARG_OFFSET + parse_int(token + 1);
			break;
			case 'r':
				return RESERVED_REG(parse_int(token + 1));
			break;
			case 'g':
				return REG_GEN_OFFSET + parse_int(token + 1);
			break;
			default:
				fprintf(stderr, "Unknown register '%s'\n", token);
			break;
		}
	}
}

int parse_token(char *token, enum addr_mode *mode)
{
	int val;

	print_dbg("Parse token: [%s]\n", token);

	switch(*token) {
		case '%':
			val = parse_reg(++token);
			*mode = MODE_REGISTER; 
		break;
		case '$':
			val = parse_int(++token);
			*mode = MODE_CONSTANT;
		break;
		/*case '!':
			val = get_sub_addr(++token);
			*mode = MODE_CONSTANT;
		break;*/
		/* Since addr of function is not yet known, assembler will later go through each jmp instruction and fill in values */
		case '!':
			val = us_count;
			*mode = MODE_CONSTANT;	
			
			unk_subs[us_count] = malloc(strlen(token));
			memset(unk_subs[us_count], 0, strlen(token));
			strcpy(unk_subs[us_count++], ++token);
		break;
		case '@':
			token++;

			if(*token == '%' || *token == '$') {
				val = *token == '%' ? parse_reg(++token) : parse_int(++token);
				*mode = *(--token) == '$' ? MODE_ADDR : MODE_ADDR_REG; 
			} else {		
				//val = lookup_addr(token);
				val = ua_count;
				*mode = MODE_ADDR;
				print_dbg("Address mode: token=[%s] val=%d\n", token, val);

				unk_addrs[ua_count] = malloc(strlen(token) + 1);
				memset(unk_addrs[ua_count], 0, strlen(token) + 1);
				strcpy(unk_addrs[ua_count++], token);
			}
		break;
		default:
			//val = lookup_addr(++token);
			//*mode = MODE_ADDR;
			/* All data values are constants, so store as constant instead */
			val = lookup_val(++token); // Will return addr if data type is string
			*mode = MODE_CONSTANT;
		break;
	}

	return val;
}

int64_t
asm_instr(char *line)
{
	char *tokens[128];
	int count = 0; 
	
	print_dbg("line: [%s]\n", line);
	
	tokens[count] = strtok(line, " ");
	
	while(tokens[count] != NULL) {
		print_dbg("tokens[%d]: %s\n", count, tokens[count]);
		tokens[++count] = strtok(NULL, " ");
	}

	char opcode = -1;
	char reg = -1, reg1 = -1;
	int val = -1;
	int format = 0;

	if(count > 1) {
		reg = atoi(tokens[1] + 1);
		
		if(count > 2) {
			reg1 = atoi(tokens[2] + 1);
		
			if(strncmp(tokens[2], "0x", 2) == 0) {
				val = strtol(tokens[2], NULL, 0);
			} else {
				val = atoi(tokens[2]);
			}
		}
	}

	if(strncmp(tokens[0], "halt", 4) == 0) {
		opcode = HALT;
		format = 0;
	} else if(strncmp(tokens[0], "ldc", 3) == 0) {
		opcode = LDC;
		format = 1;
	} else if(strncmp(tokens[0], "mov", 3) == 0) {
		opcode = MOV;
		format = 4;
	} else if(strncmp(tokens[0], "pushc", 5) != 0 && strncmp(tokens[0], "push", 4) == 0) {
		opcode = PUSH;
		format = 2;
	} else if(strncmp(tokens[0], "pop", 3) == 0) {
		opcode = POP;
		format = 2;
	} else if(strncmp(tokens[0], "pushc", 5) == 0) {
		opcode = PUSHC;
		format = 3;
		reg = -1;
		
		if(strncmp(tokens[1], "0x", 2) == 0) {
			val = strtol(tokens[1], NULL, 0);
		} else {
			val = atoi(tokens[1]);
		}
	} else if(strncmp(tokens[0], "add", 3) == 0) {
		opcode = ADD;
		format = 4;
	} else if(strncmp(tokens[0], "sub", 3) == 0) {
		opcode = SUB;
		format = 4;
	} else if(strncmp(tokens[0], "mul", 3) == 0) {
		opcode = MUL;
		format = 4;
	} else if(strncmp(tokens[0], "div", 3) == 0) {
		opcode = DIV;
		format = 4;
	} else if(strncmp(tokens[0], "mod", 3) == 0) {
		opcode = MOD;
		format = 4;
	} else if(strncmp(tokens[0], "xor", 3) == 0) {
		opcode = XOR;
		format = 4;
	} else if(strncmp(tokens[0], "or", 2) == 0) {
		opcode = OR;
		format = 4;
	} else if(strncmp(tokens[0], "shl", 3) == 0) {
		opcode = SHL;
		format = 4;
	} else if(strncmp(tokens[0], "shr", 3) == 0) {
		opcode = SHR;
		format = 4;
	} else if(strcmp(tokens[0], "print") == 0) { // print, printr, prints
		opcode = PRINT;
		format = 2;
	} else if(strncmp(tokens[0], "cmp", 3) == 0) {
		opcode = CMP;
		format = 4;
	} else if(strncmp(tokens[0], "jmp", 3) == 0) {
		opcode = JMP;
		format = 2;
	} else if(strncmp(tokens[0], "je", 2) == 0) {
		opcode = JE;
		format = 2;
	} else if(strncmp(tokens[0], "jne", 3) == 0) {
		opcode = JNE;
		format = 2;
	} else if(strcmp(tokens[0], "jl") == 0) {
		opcode = JL;
		format = 2;
	} else if(strncmp(tokens[0], "jle", 3) == 0) {
		opcode = JLE;
		format = 2;
	} else if(strcmp(tokens[0], "jg") == 0) {
		opcode = JG;
		format = 2;
	} else if(strncmp(tokens[0], "jge", 3) == 0) {
		opcode = JGE;
		format = 2;
	} else if(strncmp(tokens[0], "geti", 4) == 0) {
		opcode = GETI;
	} else if(strncmp(tokens[0], "rand", 4) == 0) {
		opcode = RAND;
	} else if(strcmp(tokens[0], "printc") == 0) {
		opcode = PRINTC;
	} else if(strcmp(tokens[0], "inc") == 0) {
		opcode = INC;
	} else if(strcmp(tokens[0], "dec") == 0) {
		opcode = DEC;
	}

	if(opcode == -1) {
		return -1;
	}

	format = get_opcode_format(opcode);

	int64_t instr  = ((int64_t)opcode) << 56;
	enum addr_mode mode[5];
	int pval[5];

	switch(format) {
		case 0:
		break;
		case 1: // Tested - works
			pval[0] = parse_token(tokens[1], &mode[0]);

			instr |= ((int64_t)pval[0] & get_mode_mask(mode[0])) << (56 - get_mode_size(mode[0]));
			
			instr |= mode[0] & 0xF;

			print_dbg("Value: %d (mode: %d) (mode mask: %016llx)\n", pval[0], mode[0], get_mode_mask(mode[0]));
		break;
		case 2:
			pval[0] = parse_token(tokens[1], &mode[0]);
			pval[1] = parse_token(tokens[2], &mode[1]);
			
			instr |= ((int64_t)pval[0] & get_mode_mask(mode[0])) << (56 - get_mode_size(mode[0]));
			instr |= ((int64_t)pval[1] & get_mode_mask(mode[1])) << (56 - get_mode_size(mode[0]) -  get_mode_size(mode[1]));
			
			instr |= mode[0] & 0xF;
			instr |= (mode[1] & 0xF) << 4;

			print_dbg("Two operand instruction: 0x%016llx\n", instr);
			print_dbg("Values: %d (%08x) (mode: %d), %d (%08x) (mode: %d)\n", pval[0], pval[0], mode[0], pval[1], pval[1], mode[1]);
		break;
	}

	print_dbg("Assembled: %016llx - Instruction: %s (opcode: %d)\n", instr, tokens[0], opcode);

	return instr;
}


