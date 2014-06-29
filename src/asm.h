#ifndef _ASM_H
#define _ASM_H

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "asm_code.h"
#include "bin.h"
#include "common.h"
#include "dbg.h"
#include "instr.h"
#include "vm.h"

void write_bin(char *fn, int64_t *buf, int bufsize, int stack_size);

#endif

