#ifndef _ll_h_
#define _ll_h_

#include <context.h>

vword_t ll_eval(Context* context, vword_t p);
vword_t ll_deval(Context* context, vword_t p);
vword_t ll_nop(Context* context, vword_t p);

vword_t ll_eq(Context* context, vword_t p);
vword_t ll_gtb(Context* context, vword_t p);

vword_t ll_map(Context* context, vword_t p);
vword_t ll_cond(Context* context, vword_t p);

vword_t ll_exit(Context* context, vword_t p);
vword_t ll_printi(Context* context, vword_t p);
vword_t ll_printb(Context* context, vword_t p);
vword_t ll_addi(Context* context, vword_t p);

// brainfuck
vword_t ll_bf_case(Context* context, vword_t p);
vword_t ll_bf_in(Context* context, vword_t p);
vword_t ll_bf_out(Context* context, vword_t p);
vword_t ll_bf_prev(Context* context, vword_t p);
vword_t ll_bf_next(Context* context, vword_t p);
vword_t ll_bf_inc(Context* context, vword_t p);
vword_t ll_bf_dec(Context* context, vword_t p);

#define LL_STUB(name, fn) \
vword_t name (Context* context, vword_t p) \
{ \
	int len = fn (context->base(p)); \
	return (p-(vword_t)len); \
}

#endif

