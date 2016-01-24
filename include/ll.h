#ifndef _ll_h_
#define _ll_h_

#include <context.h>

vword_t ll_eval(Context* context, vword_t p);
vword_t ll_deval(Context* context, vword_t p);

#define LL_STUB(name, fn) \
vword_t name (Context* context, vword_t p) \
{ \
	int len = fn (context->base(p)); \
	return p-len; \
}

#endif

