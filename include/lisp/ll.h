#ifndef _lisp_ll_h_
#define _lisp_ll_h_

#include <context.h>

void init_lisp(Context* context);

vword_t ll_quote(Context* context, vword_t p);
vword_t ll_asm(Context* context, vword_t p);

vword_t ll_expand(Context* context, vword_t p);
vword_t expand(Context* context, vword_t pt, vword_t* p, bool quoted, bool quoteptr);

vword_t ll_eval_param(Context* context, vword_t p);
vword_t ll_expand_macro(Context* context, vword_t p);

vword_t ll_function(Context* context, vword_t p);
vword_t ll_macro(Context* context, vword_t p);

vword_t ll_declare(Context* context, vword_t p);
vword_t ll_isdef(Context* context, vword_t p);

vword_t ll_lmap(Context* context, vword_t p);
vword_t ll_progn(Context* context, vword_t p);

#endif

