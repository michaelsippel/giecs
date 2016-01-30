#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>

vword_t ll_cond(Context* context, vword_t p)
{
    vbyte_t* ptr = (vbyte_t*) context->base(p);
    vbyte_t c = *ptr++;
    vword_t* v = (vword_t*) context->base(p + 1);

    if(c)
        v[1] = v[0];

    return (vword_t) p + 1 + sizeof(vword_t);
}

vword_t ll_eq(Context* context, vword_t p)
{
    vword_t* v = (vword_t*) context->base(p);
    vword_t v1 = *v++;
    vword_t v2 = *v++;

    vbyte_t* r = (vbyte_t*) v;
    *--r = (v1 == v2) ? 1 : 0;

    return (vword_t) p + 2*sizeof(vword_t) - sizeof(vbyte_t);
}

vword_t ll_map(Context* context, vword_t p)
{

}

