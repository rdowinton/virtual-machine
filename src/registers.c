#include "registers.h"

unsigned int get_register(enum register_type rt, int num)
{
	switch(rt) {
		case REGTYPE_GENERAL:
			return REG_GEN_OFFSET + num;
		break;
		case REGTYPE_RESERVED:
			return REG_RES_OFFSET + num;
		break;
		case REGTYPE_ARGUMENT:
			return REG_ARG_OFFSET + num;
		break;
	}
}

