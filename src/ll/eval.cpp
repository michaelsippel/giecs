#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>

vword_t ll_eval(Context* context, vword_t p)
{
    vword_t addr = *((vword_t*) context->base(p));
    p += sizeof(vword_t);

    vword_t* in = (vword_t*) context->base(addr);
    vword_t len = *in++;

    if(len != (vword_t)-1)
    {
        // apply parameters
        vbyte_t* src = (vbyte_t*) context->base(addr + sizeof(vword_t) + len);
        vbyte_t* dest = (vbyte_t*) context->base(p);

        int i;
        for(i = 0; i < len; i++)
        {
            *--dest = *--src;
        }

        // eval again
        return ll_eval(context, p-len);
    }
    else
    {
        // call low-level function
        vword_t (*fn)(Context*, vword_t) = (vword_t (*)(Context*, vword_t)) *((void**) in);
        return fn(context, p);
    }
}

vword_t ll_deval(Context* context, vword_t p)
{
    vword_t* in = (vword_t*) context->base(p);
    vword_t addr_fn = in[0];
    vword_t num_list = in[1];
    vword_t addr_list = in[2];

    p += 3 * sizeof(vword_t);

    // TODO: without malloc?
    void** list_index = (void**) malloc(num_list * sizeof(void*));
    uintptr_t ptr = (uintptr_t) context->base(addr_list);

    int i;
    for(i = 0; i < num_list; i++)
    {
        vword_t attr = *((vword_t*)ptr);
        list_index[i] = (void*) ptr;

        ptr += (attr == (vword_t)-1) ? sizeof(vword_t) : attr;
    }

    for(i = num_list-1; i >= 0; i--)
    {
        vword_t* list = (vword_t*) list_index[i];
        vword_t attr = *list++;

        if(attr == (vword_t)-1)
        {
            // execute
            p -= sizeof(vword_t);
            *((vword_t*)context->base(p)) = *list;

            p = ll_eval(context, p);
        }
        else
        {
            p -= attr;
            memcpy(context->base(p), (void*) list, attr);
        }
    }

    free(list_index);

    p -= sizeof(vword_t);
    *((vword_t*) context->base(p)) = addr_fn;

    return ll_eval(context, p);
}

