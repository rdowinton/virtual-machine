#ifndef _ASM_DATA_H
#define _ASM_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bin.h"
#include "common.h"

int lookup_val(char *var_name);
int lookup_addr(char *var_name);
unsigned int parse_str(char *src, char *dest, int max);
char *asm_data(char *line, DATA_ENTRY *entry, char **name_out);

#endif

