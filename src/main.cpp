#include <stdio.h>

#include <context.h>

int ll_add(void* in, void* out)
{
    *((int*)out) = ((int*)in)[0] + ((int*)in)[1];

    return sizeof(int);
}

int ll_printi(void* in, void* out)
{
    printf("%d\n", *((int*)in));
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

    uint32_t* ptr = (uint32_t*) context->base(0xa2);

    *ptr++ = 23;
    *ptr++ = 11;

    stub_ll_add(context, 0xa2, 0xc2);
    stub_ll_printi(context, 0xc2, 0x0);

    delete context;

    return 0;
}

