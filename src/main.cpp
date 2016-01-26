#include <stdio.h>

#include <ll.h>
#include <context.h>
#include <reader.h>
#include <parser.h>

int ll_addi(void* p)
{
    vword_t* in = (vword_t*) p;
    vword_t a = in[0];
    vword_t b = in[1];

    in[1] = a + b;

    return (-sizeof(vword_t));
}

int ll_printi(void* p)
{
    printf("%d\n", ((vword_t*)p)[0]);
    return (-sizeof(vword_t));
}

LL_STUB(stub_ll_addi, ll_addi);
LL_STUB(stub_ll_printi, ll_printi);

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    SNode* sn = new SNode(LIST, (char*) "12 deval printi (1 -1 (12 addi 2 3))");
    sn->dump();

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
    *ptr++ = -1;
    *ptr++ = 0x180;

    ptr = (vword_t*) context->base(0x180);
    *ptr++ = 12;
    *ptr++ = addr_addi;
    *ptr++ = 1234;
    *ptr++ = 2345;

    // set entry point
    ptr = (vword_t*) context->base(0x1000-0x4);
    *ptr++ = 0x100;
    ll_eval(context, 0x1000-0x4);

    delete context;

    return 0;
}

