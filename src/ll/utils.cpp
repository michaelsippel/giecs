#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <ll.h>

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
    vword_t* v = (vword_t*) context->base(p);
    vword_t fn_addr = *v++;
    vword_t list_len = *v++;
    vword_t entry_size = *v++;
    vword_t list_addr = *v++;

    p += 4 * sizeof(vword_t);

    vbyte_t* list = (vbyte_t*) context->base(list_addr);
    vbyte_t* dest = (vbyte_t*) v;

    int i,j;
    for(i = 0; i < list_len; i++)
    {
        for(j = entry_size-1; j >= 0; j--)
        {
            *--dest = list[i*entry_size + j];
        }
        v = (vword_t*) dest;
        *--v = fn_addr;
        *--v = entry_size + sizeof(vword_t);
        p -= entry_size + 2*sizeof(vword_t);
        p = ll_eval(context, p);
    }
    return p;
}

