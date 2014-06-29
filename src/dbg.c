#include "dbg.h"

#ifndef DEBUG_MODE
void print_dbg(char *fmt, ...) {}
#else
void print_dbg(char *fmt, ...)
{
	char *myfmt = (char*)malloc(32 + strlen(fmt));
	
	sprintf(myfmt, "Debug: %s", fmt);

	va_list args;
	va_start(args, fmt);
	vprintf(myfmt, args);
	va_end(args);
}
#endif

