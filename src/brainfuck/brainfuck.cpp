#include <unistd.h>
#include <string.h>

#include <ll.h>
#include <context.h>
#include <lisp/parser.h>
#include <brainfuck/parser.h>

vword_t ll_bf_case(Context* context, vword_t p)
{
    vword_t addr = (vword_t) context->read_word(p);
    vbyte_t buf;

    vword_t mptr = context->read_word(resolve_symbol("bf_memptr").start);
    context->read(mptr, 1, &buf);

    //printf("mptr: 0x%x, buf: %d\n", mptr, buf);

    if(buf > (vbyte_t)0)
    {
        context->write_word(p, addr);
        p = ll_eval(context, p);
    }

    return p;
}

vword_t ll_bf_in(Context* context, vword_t p)
{
    vword_t mptr = context->read_word(resolve_symbol("bf_memptr").start);
    vbyte_t buf;
    read(0, &buf, 1);
    context->write(mptr, 1, &buf);

    return p;
}

vword_t ll_bf_out(Context* context, vword_t p)
{
    vword_t mptr = context->read_word(resolve_symbol("bf_memptr").start);
    vbyte_t buf;
    context->read(mptr, 1, &buf);
    write(1, &buf, 1);

    return p;
}

vword_t ll_bf_prev(Context* context, vword_t p)
{
    vword_t mptr = context->read_word(resolve_symbol("bf_memptr").start);
    context->write_word(resolve_symbol("bf_memptr").start, mptr - 1);
    return p;
}

vword_t ll_bf_next(Context* context, vword_t p)
{
    vword_t mptr = context->read_word(resolve_symbol("bf_memptr").start);
    context->write_word(resolve_symbol("bf_memptr").start, mptr + 1);
    return p;
}

vword_t ll_bf_inc(Context* context, vword_t p)
{
    vword_t mptr = context->read_word(resolve_symbol("bf_memptr").start);
    vbyte_t buf;
    context->read(mptr, 1, &buf);
    buf++;
    context->write(mptr, 1, &buf);

    return p;
}

vword_t ll_bf_dec(Context* context, vword_t p)
{
    vword_t mptr = context->read_word(resolve_symbol("bf_memptr").start);
    vbyte_t buf;
    context->read(mptr, 1, &buf);
    buf--;
    context->write(mptr, 1, &buf);

    return p;
}

void init_brainfuck(Context* context)
{
    add_symbol("brainfuck", context->add_ll_fn(ll_parse_brainfuck), VWORD_SIZE);

    add_symbol("bf_case", context->add_ll_fn(ll_bf_case));
    add_symbol("bf_in", context->add_ll_fn(ll_bf_in));
    add_symbol("bf_out", context->add_ll_fn(ll_bf_out));
    add_symbol("bf_next", context->add_ll_fn(ll_bf_next));
    add_symbol("bf_prev", context->add_ll_fn(ll_bf_prev));
    add_symbol("bf_inc", context->add_ll_fn(ll_bf_inc));
    add_symbol("bf_dec", context->add_ll_fn(ll_bf_dec));

    add_symbol("bf_memptr", 0x20000-VWORD_SIZE);

    context->write_word(resolve_symbol("bf_memptr").start, 0x20000);
    int i;
    for(i = 0; i < 8192; i++)
        context->write_word(0x20000 + i*VWORD_SIZE, 0);
}

int parse_brainfuck(Context* context, vword_t addr, char* prg)
{
    size_t len = strlen(prg);
    return parse_brainfuck(context, addr, prg, prg + len);
}

int parse_brainfuck_loop(Context* context, vword_t addr, char* ptr, char* end)
{
    size_t len = 3 * VWORD_SIZE;
    vword_t sub_ptr = addr+len;

    vword_t v[3];
    v[0] = 2 * VWORD_SIZE;
    v[1] = resolve_symbol("bf_case").start;
    v[2] = sub_ptr;

    len += parse_brainfuck(context, sub_ptr, ptr, end);

    vword_t naddr = sub_ptr + 3 * VWORD_SIZE;
    int n = context->read_word(naddr);
    context->write_word(naddr, n+1);

    context->write_word(addr+len, addr);
    len += VWORD_SIZE;

    context->write(addr, 3 * VWORD_SIZE, (vbyte_t*) &v);

    return len;
}

int parse_brainfuck(Context* context, vword_t addr, char* prg, char* end)
{
    size_t n = (size_t)end - (size_t)prg;
    size_t len = 6 * VWORD_SIZE;

    vword_t* pv = (vword_t*) malloc(n * sizeof(vword_t));

    int m = 0;
    int j = 0;
    char* start = NULL;

    while(prg < end)
    {
        switch(*prg)
        {
            case ',':
                if(m == 0)
                    pv[j++] = resolve_symbol("bf_in").start;
                break;

            case '.':
                if(m == 0)
                    pv[j++] = resolve_symbol("bf_out").start;
                break;

            case '>':
                if(m == 0)
                    pv[j++] = resolve_symbol("bf_next").start;
                break;

            case '<':
                if(m == 0)
                    pv[j++] = resolve_symbol("bf_prev").start;
                break;

            case '+':
                if(m == 0)
                    pv[j++] = resolve_symbol("bf_inc").start;
                break;

            case '-':
                if(m == 0)
                    pv[j++] = resolve_symbol("bf_dec").start;
                break;

            case '[':
                if(m == 0)
                    start = prg+1;
                m++;
                break;

            case ']':
                if(m == 1)
                {
                    vword_t sub_ptr = addr + len;
                    len += parse_brainfuck_loop(context, sub_ptr, start, prg);

                    pv[j++] = sub_ptr;

                    start = NULL;
                }
                m--;
                break;
        }

        prg++;
    }

    vword_t sub_ptr = addr+len;
    len += context->write(sub_ptr, j * VWORD_SIZE, (vbyte_t*) pv);
    free(pv);

    vword_t v[6];
    v[0] = 5 * VWORD_SIZE;
    v[1] = resolve_symbol("map").start;
    v[2] = resolve_symbol("eval").start;
    v[3] = j;
    v[4] = VWORD_SIZE;
    v[5] = sub_ptr;
    context->write(addr, 6*VWORD_SIZE, (vbyte_t*) &v);

    return len;
}

vword_t ll_parse_brainfuck(Context* context, vword_t p)
{
    vword_t straddr = context->read_word(p);
    char prg[512];
    context->read(straddr, 512, (vbyte_t*) &prg);

    context->write_word(resolve_symbol("bf_memptr").start, 0x20000);
    int i;
    for(i = 0; i < 8192; i++)
        context->write_word(0x20000 + i*VWORD_SIZE, 0);

    context->write_word(p, 0x800);
    parse_brainfuck(context, 0x800, prg);

    return p;
}

