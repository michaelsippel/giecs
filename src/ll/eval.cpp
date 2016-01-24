#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>

vword_t ll_eval(Context* context, vword_t p)
{
    vword_t* in = (vword_t*) context->base(p);
    vword_t addr = in[0];
    p += sizeof(vword_t);

    in = (vword_t*) context->base(addr);
    vword_t len = in[0];

    if(len != -1)
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
        vword_t (*fn)(Context*, vword_t) = (vword_t (*)(Context*, vword_t)) in[1];
        return fn(context, p);
    }
}

vword_t ll_deval(Context* context, vword_t p)
{
    vword_t* in = (vword_t*) context->base(p);
    vword_t addr_fn = in[0];
    vword_t addr_list = in[1];

    p += 2 * sizeof(vword_t);

    vword_t* list = (vword_t*) context->base(addr_list);
    vword_t num_list = *list++;

    void** list_index = (void**) malloc(num_list * sizeof(void*));
    int i;
    for(i = 0; i < num_list; i++)
    {
        vword_t attr = *list;

        list_index[i] = (void*) list++;
        list += (attr == -1) ? sizeof(vword_t) : attr;
    }

    for(i = num_list-1; i >= 0; i--)
    {
        vword_t* list = (vword_t*) list_index[i];
        vword_t attr = *list++;

        if(attr == -1)
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

