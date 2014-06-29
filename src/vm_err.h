#ifndef _VM_ERR_H
#define _VM_ERR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum error_code {
	VM_ERR_FATAL	= 0
};

void vm_error(enum error_code err, char *fmt, ...);

#endif

