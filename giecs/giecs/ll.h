#ifndef _ll_h_
#define _ll_h_

#include <context.h>
#include <stackframe.h>

void ll_eval(StackFrame& stack);

void ll_deval(StackFrame& stack);
void ll_nop(StackFrame& stack);

void ll_setrelative(StackFrame& stack);
void ll_pop(StackFrame& stack);

void ll_load(StackFrame& stack);
void ll_syscall(StackFrame& stack);

void ll_eqw(StackFrame& stack);
void ll_eqb(StackFrame& stack);
void ll_gtb(StackFrame& stack);

void ll_resw(StackFrame& stack);
void ll_setw(StackFrame& stack);
void ll_resb(StackFrame& stack);
void ll_setb(StackFrame& stack);
void ll_map(StackFrame& stack);
void ll_cond(StackFrame& stack);

void ll_printi(StackFrame& stack);
void ll_printi(StackFrame& stack);
void ll_printb(StackFrame& stack);

void ll_addi(StackFrame& stack);
void ll_addi(StackFrame& stack);
void ll_subi(StackFrame& stack);
void ll_muli(StackFrame& stack);
void ll_divi(StackFrame& stack);

#endif

