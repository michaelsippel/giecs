#include <unistd.h>
#include <string.h>

#include <ll.h>
#include <context.h>
#include <parser.h>
/*
vword_t ll_bf_case(Context* context, vword_t p)
{
    vword_t* v = (vword_t*) context->base(p);
    vword_t addr = *v;

    vword_t mptr;
    vbyte_t* buf;

begin:
    mptr = *((vword_t*) context->base(resolve_symbol("bf_memptr")));
    buf = (vbyte_t*) context->base(mptr);

    if((*buf) > (vbyte_t)0)
    {
        *((vword_t*) context->base(p)) = addr;
        ll_eval(context, p);
        goto begin;
    }

    return p;
}

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

void init_brainfuck(Context* context)
{
    add_symbol("brainfuck", context->add_ll_fn(ll_parse_brainfuck));

    add_symbol("bf_case", context->add_ll_fn(ll_bf_case));
    add_symbol("bf_in", context->add_ll_fn(ll_bf_in));
    add_symbol("bf_out", context->add_ll_fn(ll_bf_out));
    add_symbol("bf_next", context->add_ll_fn(ll_bf_next));
    add_symbol("bf_prev", context->add_ll_fn(ll_bf_prev));
    add_symbol("bf_inc", context->add_ll_fn(ll_bf_inc));
    add_symbol("bf_dec", context->add_ll_fn(ll_bf_dec));

    add_symbol("bf_memptr", 0x4fc);

    *((vword_t*) context->base(resolve_symbol("bf_memptr"))) = 0x500;
    vbyte_t* bf_mem = (vbyte_t*) context->base(0x500);
    memset(bf_mem, 0, 0x100);
}

int parse_brainfuck(Context* context, vword_t addr, char* prg)
{
    size_t len = strlen(prg);
    return parse_brainfuck(context, addr, prg, prg + len);
}

int parse_brainfuck(Context* context, vword_t addr, char* prg, char* end)
{
    size_t n = (size_t)end - (size_t)prg;
    size_t len = 0;

    vword_t* v = (vword_t*) context->base(addr);

    len += 6*sizeof(vword_t);

    *v++ = 5 * sizeof(vword_t);
    *v++ = resolve_symbol("map");
    *v++ = resolve_symbol("eval");
    vword_t* nn = v++;
    *v++ = sizeof(vword_t);
    *v++ = addr + len;

    len += n * sizeof(vword_t);

    vword_t* ve = &v[n];

    int m = 0;
    char* start = NULL;
    while(prg < end)
    {
        switch(*prg)
        {
            case ',':
                if(m == 0)
                    *v++ = resolve_symbol("bf_in");
                break;

            case '.':
                if(m == 0)
                    *v++ = resolve_symbol("bf_out");
                break;

            case '>':
                if(m == 0)
                    *v++ = resolve_symbol("bf_next");
                break;

            case '<':
                if(m == 0)
                    *v++ = resolve_symbol("bf_prev");
                break;

            case '+':
                if(m == 0)
                    *v++ = resolve_symbol("bf_inc");
                break;

            case '-':
                if(m == 0)
                    *v++ = resolve_symbol("bf_dec");
                break;

            case '[':
                if(m == 0)
                    start = prg+1;
                m++;
                break;

            case ']':
                if(m == 1)
                {
                    *v++ = addr+len;

                    vword_t* sv = (vword_t*) context->base(addr+len);

                    len += 3 * sizeof(vword_t);
                    *sv++ = 2 * sizeof(vword_t);
                    *sv++ = resolve_symbol("bf_case");
                    *sv++ = addr + len;

                    len += parse_brainfuck(context, addr+len, start, prg);

                    start = NULL;
                }
                m--;
                break;
        }

        prg++;
    }

    *nn = n - (vword_t)( ((uintptr_t)ve - (uintptr_t)v) / sizeof(vword_t) );

    return len;
}

vword_t ll_parse_brainfuck(Context* context, vword_t p)
{
    vword_t* v = (vword_t*) context->base(p);
    char* prg = (char*) context->base(*v);

    *((vword_t*) context->base(resolve_symbol("bf_memptr"))) = 0x500;
    vbyte_t* bf_mem = (vbyte_t*) context->base(0x500);
    memset(bf_mem, 0, 0x100);

    *v = 0x600; // TODO
    parse_brainfuck(context, *v, prg);

    return p;
}
*/
