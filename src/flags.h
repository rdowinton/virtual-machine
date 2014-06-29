#ifndef _FLAGS_H
#define _FLAGS_H

#define FLAG_ZERO	0

#define GET_FLAG(flag) (cpu->registers[RESERVED_REG(REG_FLAGS)] & (1 << flag))

#endif

