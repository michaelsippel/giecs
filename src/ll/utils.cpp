#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <ll.h>

vword_t ll_cond(Context* context, vword_t p)
{
    vbyte_t c;
    context->read(p, 1, &c);
    vword_t v0 = context->read_word(p + 1);

    if(c)
        context->write_word(p + 1 + VWORD_SIZE, v0);

    return p + 1 + VWORD_SIZE;
}

vword_t ll_eq(Context* context, vword_t p)
{
    vword_t v[2];
    context->read(p, 2*VWORD_SIZE, (vbyte_t*) &v);

    p = p + 2 * VWORD_SIZE - 1;

    vbyte_t r = (v[0] == v[1]) ? 1 : 0;
    context->write(p, 1, &r);

    return p;
}

vword_t ll_map(Context* context, vword_t p)
{
    vword_t v[4];
    context->read(p, 4 * VWORD_SIZE, (vbyte_t*) &v);

    vword_t fn_addr = v[0];
    vword_t list_len = v[1];
    vword_t entry_size = v[2];
    vword_t list_addr = v[3];

    p += 4 * VWORD_SIZE;

    size_t l = list_len * entry_size;

    vbyte_t* list = (vbyte_t*) malloc(l * sizeof(vbyte_t));
    context->read(list_addr, l, list);

    vbyte_t* dest = (vbyte_t*) malloc(entry_size * sizeof(vbyte_t));

    int i,j;
    for(i = 0; i < list_len; i++)
    {
        vword_t* h = (vword_t*) dest;
        *h++ = entry_size + VWORD_SIZE;
        *h++ = fn_addr;

        vbyte_t* d = dest + 2*VWORD_SIZE;
        for(j = 0; j < entry_size; j++)
        {
            *d++ = list[i*entry_size + j];
        }

        p -= entry_size + 2*VWORD_SIZE;
        context->write(p, entry_size + 2*VWORD_SIZE, dest);

        p = ll_eval(context, p);
    }

    free(dest);
    free(list);

    return p;
}

