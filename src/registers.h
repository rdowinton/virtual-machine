#ifndef _REGISTERS_H
#define _REGISTERS_H

#define REG_GEN_OFFSET 0
#define REG_RES_OFFSET 16
#define REG_ARG_OFFSET 32

#define REG_IP		0
#define REG_BASEP	1
#define REG_STACKP	2
#define REG_RADDR	3
#define REG_RVAL	4
#define REG_MATH	5
#define REG_CMP		6
#define REG_FLAGS	7
#define REG_OUTPUT	8

#define GENERAL_REG(r) (r + REG_GEN_OFFSET)
#define RESERVED_REG(r) (r + REG_RES_OFFSET)
#define ARG_REG(r) (r + REG_ARG_OFFSET)

enum register_type {
	REGTYPE_GENERAL,
	REGTYPE_RESERVED,
	REGTYPE_ARGUMENT
};

#endif

