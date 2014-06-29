#include "vm_err.h"

void vm_error(enum error_code err, char *fmt, ...)
{
	char *myfmt = (char*)malloc(32 + strlen(fmt));
	
	sprintf(myfmt, "Error: %s", fmt);

	va_list args;
	va_start(args, fmt);
	vprintf(myfmt, args);
	va_end(args);

	switch(err) {
		case VM_ERR_FATAL:
			vm_shutdown(EXIT_FAILURE);
		break;
	}
}

