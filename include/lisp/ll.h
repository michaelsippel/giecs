#ifndef _lisp_ll_h_
#define _lisp_ll_h_

#include <context.h>

void init_lisp(Context* context);

vword_t ll_gen_fn(Context* context, vword_t p);

vword_t ll_quote(Context* context, vword_t p);

vword_t ll_declare(Context* context, vword_t p);
vword_t ll_asm(Context* context, vword_t p);

vword_t ll_add(Context* context, vword_t p);

#endif

