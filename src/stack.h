#ifndef _STACK_H
#define _STACK_H

#include "registers.h"
#include "vm.h"
#include "vm_err.h"

#define DEFAULT_STACK_SIZE 256

void push(int val);
int pop();

#endif

