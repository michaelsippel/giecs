#ifndef _brainfuck_ll_h_
#define _brainfuck_ll_h_

#include <giecs/context.h>

// brainfuck
vword_t ll_bf_case(Context* context, vword_t p);
vword_t ll_bf_in(Context* context, vword_t p);
vword_t ll_bf_out(Context* context, vword_t p);
vword_t ll_bf_prev(Context* context, vword_t p);
vword_t ll_bf_next(Context* context, vword_t p);
vword_t ll_bf_inc(Context* context, vword_t p);
vword_t ll_bf_dec(Context* context, vword_t p);

#endif

