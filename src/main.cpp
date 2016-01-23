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

int ll_add(void* in, void* out)
{
    ((int*)out)[-1] = ((int*)in)[0] + ((int*)in)[1];

    return sizeof(int);
}

int ll_printi(void* in, void* out)
{
    printf("%d\n", ((int*)in)[0]);
    return 0;
}

#define LL_STUB(name, fn) \
int name (Context* context, vword_t addr_in, vword_t addr_out) \
{ \
	int len = fn (context->base(addr_in), context->base(addr_out)); \
	/* TODO: copy output buffer*/ \
	return len; \
}

LL_STUB(stub_ll_add, ll_add);
LL_STUB(stub_ll_printi, ll_printi);

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    Context* context = new Context(4096);

    uint32_t* ptr = (uint32_t*) context->base(0x1);
    *ptr++ = -1;
    *ptr++ = (uint32_t) &stub_ll_printi;

    ptr = (uint32_t*) context->base(0xa);
    *ptr++ = -1;
    *ptr++ = (uint32_t) &stub_ll_add;

    ptr = (uint32_t*) context->base(0x20);
    *ptr++ = 12;
    *ptr++ = 0xa;
    *ptr++ = 123;
    *ptr++ = 234;

    int l = ll_eval(context, 0x20, 0x1000);
    stub_ll_printi(context, 0x1000 - 12, 0x1000);

    delete context;

    return 0;
}

