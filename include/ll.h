#ifndef _ll_h_
#define _ll_h_

#include <context.h>

vword_t ll_eval(Context* context, vword_t p);
vword_t ll_deval(Context* context, vword_t p);
vword_t ll_nop(Context* context, vword_t p);

vword_t ll_syscall(Context* context, vword_t p);

vword_t ll_eq(Context* context, vword_t p);
vword_t ll_gtb(Context* context, vword_t p);

vword_t ll_resw(Context* context, vword_t p);
vword_t ll_setw(Context* context, vword_t p);
vword_t ll_map(Context* context, vword_t p);
vword_t ll_cond(Context* context, vword_t p);

vword_t ll_printi(Context* context, vword_t p);
vword_t ll_printb(Context* context, vword_t p);

vword_t ll_addi(Context* context, vword_t p);
vword_t ll_subi(Context* context, vword_t p);
vword_t ll_muli(Context* context, vword_t p);
vword_t ll_divi(Context* context, vword_t p);

#endif

