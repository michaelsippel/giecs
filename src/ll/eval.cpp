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
    /*    vword_t* in = (vword_t*) context->base(p);
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
        return ll_eval(context, p);*/
    return p;
}

