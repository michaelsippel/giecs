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
        return ll_eval(context, addr, addr_out - len + sizeof(vword_t));
    }
    else
    {
        // call low-level function
        int (*fn)(Context*, vword_t, vword_t) = (int (*)(Context*, vword_t, vword_t))addr;
        return fn(context, addr_out, addr_out);
    }
}

