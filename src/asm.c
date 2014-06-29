/*
 * Author: Richard Dowinton
 * 
 * Assembles bytecode for VM
 *
 * Modes:
 *
 * % = register
 * $ = constant
 * @ = data located at address
 * @% = data located at the address stored in register
 * ! = subroutine (assembled is just a constant address)
 * <none> = variable
 *
 * Registers:
 *
 * %g0 .. %g15 = general
 * %a0 .. %a31 = command line arguments (pointers)
 * %r0 .. %r15 = reserved (r8 = math, r9 = stack pointer)
 * %bp = base pointer
 * %ip = instruction pointer (program counter)
 * %sp = stack pointer
 * %m = math output
 * %rv = return value
 * %ra = return address
 * %cmp = comparison output
 *
 * Instructions:
 *
 * halt;
 * ldc %g1 $5;
 * mov %g1 %g2; mov $1000 %g1; mov i %g2; mov @str %g3; # mov <src> <dest>, non register as dest will cause error
 * push %g1; push $1; push i; push str;
 * pop %g1;
 * add %g1 %g2; add %g1 $1; add %g1 i;
 * sub, mul, div, mod, xor, or, shl, shr follow same format as add
 * print %g1; print @%g1; print $1; print i; print str;
 * jmp !sub; jmp addr; jmp @%g1; jmp $200;
 * je, jne, jl, jle, jg, jge follow same format as jmp
 */

#include "asm.h"

void write_int64(char *buf, int *offset, int64_t i)
{
	char *p = (buf + *offset);
	memcpy(p, &i, sizeof(int64_t));
	*offset += sizeof(int64_t);
}

unsigned int
parse_lines(BIN_DATA *bin_data, char *lines, char *buf, unsigned int bufsize)
{
	SECTION_HDR **sections = bin_data->sections;
	DATA_ENTRY **data_entries = bin_data->data_entries;
	char **data_names = bin_data->data_names;
	SUBROUTINE_HDR **subs = bin_data->subs;

	us_count = 0;
	unk_subs = malloc(sizeof(int) * 4096);
	ua_count = 0;
	unk_addrs = malloc(sizeof(int) * 4096);

	unsigned int *addr_refs = malloc(sizeof(int) * 4096);
	unsigned int aref_c = 0;

	unsigned int *jmps = malloc(sizeof(int) * 4096);
	unsigned int offset = 0, datac = 0, subidx = 0, jmpc = 0;
	char *p = lines;
	int sectidx = -1;
	int64_t instr;

	while(*p) {
		char *tmp = strstr(p, ";");
		
		if(tmp == NULL) break;
		
		*tmp = 0;
		
		p = trim_ws(p);
		
		if(*p == '.') {
			p++;

			if(strncmp(p, "section", 7) == 0) {
				p += 7;

				p = trim_ws(p);

				if(sectidx != -1) {
					sections[sectidx]->size = offset - sections[sectidx]->offset;
				}

				sectidx++;
				sections[sectidx] = malloc(sizeof(SECTION_HDR));
				sections[sectidx]->offset = offset;
				strncpy(sections[sectidx]->name, p, SECTION_NAME_SIZE);
			
				print_dbg("Entering section '%s'\n", sections[sectidx]->name);
			} else if(strncmp(p, "sub", 3) == 0) {
				p = trim_ws(p + 3);

				subs[subidx] = malloc(sizeof(SUBROUTINE_HDR));
				subs[subidx]->offset = offset;
				strncpy(subs[subidx]->name, p, strlen(p) < SUBROUTINE_NAME_SIZE ? strlen(p) : SUBROUTINE_NAME_SIZE);

				print_dbg("Entering subroutine '%s'\n", subs[subidx]->name);
			} else if(strncmp(p, "ends", 4) == 0) {
				subs[subidx]->size = offset - subs[subidx]->offset;
				subidx++;
			} else if(strncmp(p, "entryp", 6) == 0) {
				p = trim_ws(p + 6);

				bin_data->entryp_name = malloc(strlen(p) + 1);
				memset(bin_data->entryp_name, 0, strlen(p) + 1);
				strcpy(bin_data->entryp_name, p);
			}
		} else if(*p != '#') {
			if(sectidx == -1) {
				fprintf(stderr, "No section defined\n");
				exit(EXIT_FAILURE);
			}

			print_dbg("Parsing line: [%s]\n", p);

			char *tmp = malloc(strlen(p) + 1);
			memset(tmp, 0, strlen(p) + 1);
			strcpy(tmp, p);

			if((instr = asm_instr(tmp)) != -1) {
				if(is_jmp(instr)) {
					print_dbg("is_jmp(%016llx) = true\n", instr);
					jmps[jmpc++] = offset;
				}

				switch(get_opcode_format(GET_OPCODE(instr))) {
					case TWO_OPERANDS:
						if(GET_ADDR_MODE(instr, 1) == MODE_ADDR) {
							addr_refs[aref_c++] = offset;
						}
					case ONE_OPERAND:
						if(GET_ADDR_MODE(instr, 0) == MODE_ADDR) {
							addr_refs[aref_c++] = offset;
						}
					break;
				}

				write_int64(buf, &offset, instr);
			} else {
				DATA_ENTRY *entry = malloc(sizeof(DATA_ENTRY));
				char *data = (char*)asm_data(p, entry, &data_names[datac]);

				if(data) {
					print_dbg("data_names[%d] = %s\n", datac, data_names[datac]);
					entry->idx = datac;
					entry->offset = offset;
					data_entries[datac++] = entry;
					memcpy(&buf[offset], data, entry->size);
					offset += entry->size;
				} else {
					free(entry);
				}
			}

			if(offset == bufsize - 1) {
				buf = realloc(buf, bufsize * 2);
			}
		} else if(*p == '#') {
			char *tmp2 = strstr(p, "\n");

			if(tmp2 != NULL) {
				*tmp = ';';
				tmp = tmp2;
				*tmp2 = 0;
				print_dbg("Comment: '%s'\n", ++p);
			}
		}

		p = tmp + 1;
	}

	bin_data->num_subs = subidx;
	fix_jmps(bin_data, buf, jmps, jmpc);
	fix_var_addrs(bin_data, buf, addr_refs, aref_c);
	sections[sectidx]->size = offset - sections[sectidx]->offset;
	bin_data->num_data = datac;

	return sectidx + 1;
}

void asm_file(char *ifn, char *ofn, int stack_size)
{
	struct stat s;

	if(stat(ifn, &s) != 0) {
		fprintf(stderr, "stat() failed for '%s': %s\n", ifn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	FILE *fd = fopen(ifn, "rb");
	char *lines = malloc(s.st_size);

	fread(lines, sizeof(char), (size_t)s.st_size, fd);
	fclose(fd);

	print_dbg("lines = [%s]\n", lines);
	//int64_t *buf = malloc(s.st_size);
	char *buf = malloc(s.st_size * 4);
	BIN_DATA *bin_data = malloc(sizeof(BIN_DATA));

	bin_data->sections = malloc(sizeof(int) * 100);
	bin_data->data_entries = malloc(sizeof(int) * 1000);
	bin_data->data_names = malloc(sizeof(char*) * 1000);
	bin_data->subs = malloc(sizeof(int) * 1000);

	SECTION_HDR **sections = bin_data->sections;
	DATA_ENTRY **data_entries = bin_data->data_entries;
	char **data_names = bin_data->data_names;

	unsigned int num_sections = parse_lines(bin_data, lines, buf, s.st_size);
	SECTION_HDR *section;

	unsigned int temp = 0, data_section = -1, code_section = -1, tmp_off = 0;

	print_dbg("Assembled %d sections\n", num_sections);

	while(temp < num_sections) {
		section = sections[temp++];

		if(strcmp(section->name, "data") == 0) {
			data_section = temp - 1;
		} else if(strcmp(section->name, "code") == 0) {
			code_section = temp - 1;
		}

		print_dbg("\n\tSection size: %d\n\tSection offset: %d\n\tSection name: %s\n", section->size, section->offset, section->name);
	}

	if(data_section == -1) {
		fclose(fd);
		fprintf(stderr, "Data section missing\n");
		exit(EXIT_FAILURE);
	}

	if(code_section == -1) {
		fclose(fd);
		fprintf(stderr, "Code section missing\n");
		exit(EXIT_FAILURE);
	}

	BIN_HDR *bin = malloc(sizeof(BIN_HDR));

	bin->magic = BIN_MAGIC;
	bin->version = VM_VERSION;
	bin->stack_size = stack_size;
	bin->entry_point = get_sub_addr(bin_data, bin_data->entryp_name);
	bin->num_sections = num_sections;
	bin->data_section = data_section;
	bin->code_section = code_section;
	bin->sections = (SECTION_HDR*)sizeof(BIN_HDR);

	printf("Entry point: %d\n", get_sub_addr(bin_data, bin_data->entryp_name));

	fd = fopen(ofn, "wb");
	
	fwrite(bin, sizeof(BIN_HDR), 1, fd);

	for(temp = 0; temp < num_sections; temp++) {
		fwrite(sections[temp], sizeof(SECTION_HDR), 1, fd);
		//fwrite(buf + sections[temp]->offset, sizeof(int64_t), sections[temp]->size, fd);
	}

	for(temp = 0; temp < num_sections; temp++) {
		fwrite(buf + sections[temp]->offset, sizeof(char), sections[temp]->size, fd);
		//fwrite(buf + sections[temp]->offset, sizeof(int64_t), sections[temp]->size, fd);
	}

	fclose(fd);
}

int main(int argc, char **argv)
{
	int c;
	char *ofn = "a.bin";
	char *ifn;
	int stack_size = DEFAULT_STACK_SIZE;

	while((c = getopt(argc, argv, "o:s:")) != -1) {
		switch(c) {
			case 'o':
				ofn = optarg; 
			break;
			case 's':
				stack_size = strtol(optarg, NULL, 10);

				if(stack_size <= 0) {
					fprintf(stderr, "Stack size must be positive\n");
					return 1;
				}
			break;
			case '?':
				if(optopt == 'c') {
					fprintf(stderr, "Missing argument for option -%c\n", optopt);
				} else {
					fprintf(stderr, "Unknown option -%c\n", optopt);
				}
				
				return 1;
			default:
				abort();
		}
	}

	if(optind < argc) {
		ifn = argv[optind];
	} else {
		fprintf(stderr, "No input file\n");
		return 1;
	}

	asm_file(ifn, ofn, stack_size);
}

