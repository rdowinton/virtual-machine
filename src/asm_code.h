#ifndef _ASM_CODE_H
#define _ASM_CODE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bin.h"
#include "instr.h"
#include "registers.h"

typedef struct {
	SECTION_HDR **sections;
	unsigned int num_data;
	DATA_ENTRY **data_entries;
	char **data_names;
	char *entryp_name;
	unsigned int num_subs;
	SUBROUTINE_HDR **subs;
} BIN_DATA;

extern unsigned int us_count;
extern char **unk_subs;

extern unsigned int ua_count;
extern char **unk_addrs;

int get_sub_addr(BIN_DATA *bin, char *subname);
void fix_var_addrs(BIN_DATA *bin, char *buf, unsigned int *addr_refs, unsigned int aref_c);
void fix_jmps(BIN_DATA *bin, char *buf, unsigned int *jmps, int jmpc);
int64_t asm_instr(char *line);

#endif

