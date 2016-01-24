#include <stdio.h>

#include <ll.h>
#include <context.h>

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

LL_STUB(stub_ll_addi, ll_addi);
LL_STUB(stub_ll_printi, ll_printi);

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    Context* context = new Context(4096);

    vword_t addr_eval   = context->add_ll_fn(ll_eval);
    vword_t addr_deval  = context->add_ll_fn(ll_deval);
    vword_t addr_printi = context->add_ll_fn(stub_ll_printi);
    vword_t addr_addi   = context->add_ll_fn(stub_ll_addi);

    vword_t* ptr = (vword_t*) context->base(0x100);
    *ptr++ = 12;
    *ptr++ = addr_deval;
    *ptr++ = addr_printi;
    *ptr++ = 0x150;

    ptr = (vword_t*) context->base(0x150);
    *ptr++ = 1; // number of elements
    *ptr++ = -1; // copy 4 bytes
    *ptr++ = 0x180;

    ptr = (vword_t*) context->base(0x180);
    *ptr++ = 12;
    *ptr++ = addr_addi;
    *ptr++ = 1234;
    *ptr++ = 2345;

    int l = ll_eval(context, 0x100, 0x1000);

    delete context;

    return 0;
}

