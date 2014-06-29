#include "stack.h"

void push(int val)
{
	if(cpu->registers[RESERVED_REG(REG_STACKP)] > 0) {
		cpu->stack[--cpu->registers[RESERVED_REG(REG_STACKP)]] = val;
		print_dbg("push - sp: %d\n", cpu->registers[RESERVED_REG(REG_STACKP)]);
	} else {
		vm_error(VM_ERR_FATAL, "Stack overflow - stack size: %d, sp: %d\n", cpu->stack_size, cpu->registers[RESERVED_REG(REG_STACKP)]);
	}
}

int pop()
{
	if(cpu->registers[RESERVED_REG(REG_STACKP)] < cpu->stack_size) {
		print_dbg("pop - sp: %d\n", cpu->registers[RESERVED_REG(REG_STACKP)]);
		return cpu->stack[cpu->registers[RESERVED_REG(REG_STACKP)]++];
	} else {
		vm_error(VM_ERR_FATAL, "Stack underflow - stack size: %d, sp: %d\n", cpu->stack_size, cpu->registers[RESERVED_REG(REG_STACKP)]); 
	}
}
