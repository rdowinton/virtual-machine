#include "common.h"

inline char *trim_ws(char *p)
{
//	while(isspace(*p)) p++;
	while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

	return p;
}

