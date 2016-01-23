#include <stdio.h>

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

int ll_addi(void* in, void* out)
{
    ((vword_t*)out)[-1] = ((vword_t*)in)[0] + ((vword_t*)in)[1];

    return sizeof(vword_t);
}

int ll_printi(void* in, void* out)
{
    printf("%d\n", ((vword_t*)in)[0]);
    return 0;
}

#define LL_STUB(name, fn) \
int name (Context* context, vword_t addr_in, vword_t addr_out) \
{ \
	int len = fn (context->base(addr_in), context->base(addr_out)); \
	/* TODO: copy output buffer*/ \
	return len; \
}

LL_STUB(stub_ll_addi, ll_addi);
LL_STUB(stub_ll_printi, ll_printi);

vword_t add_ll_fn(Context* context, int (*fn)(Context*, vword_t, vword_t))
{
    static vword_t addr = 0x1;
    vword_t ret = addr;

    vword_t* ptr = (vword_t*) context->base(addr);
    *ptr++ = -1;
    *ptr++ = (vword_t) fn;
    addr += 2 * sizeof(vword_t);

    return ret;
}

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    Context* context = new Context(4096);

    vword_t addr_eval   = add_ll_fn(context, ll_eval);
    vword_t addr_printi = add_ll_fn(context, stub_ll_printi);
    vword_t addr_addi   = add_ll_fn(context, stub_ll_addi);

    vword_t* ptr = (vword_t*) context->base(0x100);
    *ptr++ = 12;
    *ptr++ = addr_addi;
    *ptr++ = 123;
    *ptr++ = 234;

    int l = ll_eval(context, 0x100, 0x1000);
    stub_ll_printi(context, 0x1000 - 12, 0x1000);

    delete context;

    return 0;
}

