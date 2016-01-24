#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>

int ll_eval(Context* context, vword_t addr_in, vword_t addr_out)
{
    vword_t* in = (vword_t*) context->base(addr_in);
    vword_t len = in[0];
    vword_t addr = in[1];

    if(len != -1)
    {
        // apply parameters
        vbyte_t* src = (vbyte_t*) context->base(addr_in + sizeof(vword_t) + len);
        vbyte_t* dest = (vbyte_t*) context->base(addr_out);
        int i;
        for(i = sizeof(vword_t); i < len; i++)
        {
            *--dest = *--src;
        }

        // eval again
        return ll_eval(context, addr, addr_out - len + sizeof(vword_t)) + len - sizeof(vword_t);
    }
    else
    {
        // call low-level function
        int (*fn)(Context*, vword_t, vword_t) = (int (*)(Context*, vword_t, vword_t))addr;
        return fn(context, addr_out, addr_out);
    }
}

int ll_deval(Context* context, vword_t addr_in, vword_t addr_out)
{
    vword_t* in = (vword_t*) context->base(addr_in);
    vword_t addr_fn = in[0];
    vword_t addr_list = in[1];

    vword_t* list = (vword_t*) context->base(addr_list);
    vword_t num_list = *list++;

    uintptr_t buf = 0;
    size_t buflen = 0;

    int i;
    for(i = 0; i < num_list; i++)
    {
        vword_t attr = *list++;

        void* pbase;
        size_t plength;

        if(attr == -1)
        {
            // execute
            vword_t addr = *list++;

            plength = ll_eval(context, addr, addr_out);
            pbase = context->base(addr_out - plength);
        }
        else
        {
            pbase = list;
            plength = attr;
            list = (vword_t*) (((uintptr_t)list) + plength);
        }

        // attach new parameters
        buf = (uintptr_t) realloc((void*)buf, buflen+plength);
        memcpy((void*)(buf+buflen), pbase, plength);
        buflen += plength;
    }

    addr_out -= buflen;
    memcpy(context->base(addr_out), (void*)buf, buflen);
    return ll_eval(context, addr_fn, addr_out);
}

