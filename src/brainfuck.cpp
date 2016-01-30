#include <unistd.h>
#include <string.h>

#include <context.h>
#include <parser.h>

vword_t ll_bf_in(Context* context, vword_t p)
{
    vword_t mptr = *((vword_t*) context->base(resolve_symbol("bf_memptr")));
    vbyte_t* buf = (vbyte_t*) context->base(mptr);
    read(0, buf, 1);
    return p;
}

vword_t ll_bf_out(Context* context, vword_t p)
{
    vword_t mptr = *((vword_t*) context->base(resolve_symbol("bf_memptr")));
    vbyte_t* buf = (vbyte_t*) context->base(mptr);
    write(1, buf, 1);
    return p;
}

vword_t ll_bf_prev(Context* context, vword_t p)
{
    vword_t* mptr = ((vword_t*) context->base(resolve_symbol("bf_memptr")));
    *mptr = (*mptr) - 1;
    return p;
}

vword_t ll_bf_next(Context* context, vword_t p)
{
    vword_t* mptr = ((vword_t*) context->base(resolve_symbol("bf_memptr")));
    *mptr = (*mptr) + 1;
    return p;
}

vword_t ll_bf_inc(Context* context, vword_t p)
{
    vword_t mptr = *((vword_t*) context->base(resolve_symbol("bf_memptr")));
    vbyte_t* buf = (vbyte_t*) context->base(mptr);
    *buf = (*buf) + 1;
    return p;
}

vword_t ll_bf_dec(Context* context, vword_t p)
{
    vword_t mptr = *((vword_t*) context->base(resolve_symbol("bf_memptr")));
    vbyte_t* buf = (vbyte_t*) context->base(mptr);
    *buf = (*buf) - 1;
    return p;
}

int parse_brainfuck(Context* context, vword_t addr, char* prg)
{
    int n = strlen(prg);

    vword_t* v = (vword_t*) context->base(addr);
    *v++ = 6 * sizeof(vword_t);
    *v++ = resolve_symbol("map");
    *v++ = resolve_symbol("eval");
    *v++ = n;
    *v++ = sizeof(vword_t);
    *v++ = addr + 6*sizeof(vword_t);

    while(*prg != '\0')
    {
        vword_t a = 0;
        switch(*prg)
        {
            case ',':
                a = resolve_symbol("bf_in");
                break;

            case '.':
                a = resolve_symbol("bf_out");
                break;

            case '>':
                a = resolve_symbol("bf_next");
                break;

            case '<':
                a = resolve_symbol("bf_prev");
                break;

            case '+':
                a = resolve_symbol("bf_inc");
                break;

            case '-':
                a = resolve_symbol("bf_dec");
                break;

            case '[':
            case ']':
                break;
        }
        *v++ = a;

        prg++;
    }

    return addr;
}

