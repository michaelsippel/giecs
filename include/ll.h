#ifndef _ll_h_
#define _ll_h_

#include <context.h>

int ll_eval(Context* context, vword_t addr_in, vword_t addr_out);
int ll_deval(Context* context, vword_t addr_in, vword_t addr_out);

#define LL_STUB(name, fn) \
int name (Context* context, vword_t addr_in, vword_t addr_out) \
{ \
	int len = fn (context->base(addr_in), context->base(addr_out)); \
	/* TODO: copy output buffer*/ \
	return len; \
}



#endif

