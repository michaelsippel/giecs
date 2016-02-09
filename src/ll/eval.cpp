#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>

vword_t ll_eval(Context* context, vword_t p)
{
    vword_t addr;
    context->read(p, sizeof(vword_t), (vbyte_t*) &addr);
    p += sizeof(vword_t);

    vword_t len;
    context->read(addr, sizeof(vword_t), (vbyte_t*) &len);
    addr += sizeof(vword_t);

    if(len != (vword_t)-1)
    {
        // apply parameters
        vbyte_t* buf = (vbyte_t*) malloc(len * sizeof(vbyte_t));

        context->read(addr, len, buf);
        context->write(p-len, len, buf);

        free(buf);

        // eval again
        return ll_eval(context, p-len);
    }
    else
    {
        // call low-level function
        vword_t (*fn)(Context*, vword_t);
        context->read(addr, sizeof(void*), (vbyte_t*) &fn);

        return fn(context, p);
    }
}

vword_t ll_deval(Context* context, vword_t p)
{
    vword_t in[2];
    context->read(p, 2*sizeof(vword_t), (vbyte_t*) &in);
    vword_t num_list = in[0];
    vword_t addr_list = in[1];

    p += 2 * sizeof(vword_t);

    // TODO: without malloc?
    vword_t* list_index = (vword_t*) malloc(num_list * sizeof(void*));

    int i;
    for(i = 0; i < num_list; i++)
    {
        vword_t attr;
        context->read(addr_list, sizeof(vword_t), (vbyte_t*) &attr);

        list_index[i] = addr_list;

        addr_list += sizeof(vword_t);
        addr_list += (attr == (vword_t)-1) ? sizeof(vword_t) : attr;
    }

    for(i = num_list-1; i >= 0; i--)
    {
        vword_t list_addr = list_index[i];
        vword_t attr;
        context->read(list_addr, sizeof(vword_t), (vbyte_t*) &attr);
        list_addr += sizeof(vword_t);

        if(attr == (vword_t)-1)
        {
            // execute
            p -= sizeof(vword_t);
            vword_t w;
            context->read(list_addr, sizeof(vword_t), (vbyte_t*) &list_addr);
            context->write(p, sizeof(vword_t), (vbyte_t*) &list_addr);

            p = ll_eval(context, p);
        }
        else
        {
            p -= attr;

            vbyte_t* buf = (vbyte_t*) malloc(attr * sizeof(vbyte_t));

            context->read(list_addr, attr, buf);
            context->write(p, attr, buf);

            free(buf);
        }
    }

    free(list_index);

    return ll_eval(context, p);
}

