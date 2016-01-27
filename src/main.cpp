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

    return (int)(-sizeof(vword_t));
}

int ll_printi(void* p)
{
    printf("%d\n", ((vword_t*)p)[0]);
    return (int)(-sizeof(vword_t));
}

LL_STUB(stub_ll_addi, ll_addi);
LL_STUB(stub_ll_printi, ll_printi);

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    Context* context = new Context(0x1000);

    add_symbol("eval", context->add_ll_fn(ll_eval));
    add_symbol("deval", context->add_ll_fn(ll_deval));
    add_symbol("printi", context->add_ll_fn(stub_ll_printi));
    add_symbol("addi", context->add_ll_fn(stub_ll_addi));

    SNode* ast = new SNode(LIST, (char*) "16 deval printi 1 (-1 (12 addi 2 3))");
    ast->dump();
//	parse_list(context, 0x100, ast);

    vword_t p = 0x100;
    p += parse(context, p, new SNode(INTEGER, (char*) "8"));
    p += parse(context, p, new SNode(SYMBOL, (char*) "printi"));
    p += parse(context, p, new SNode(INTEGER, (char*) "123"));

    // set entry point
    *((vword_t*) context->base(0x1000-sizeof(vword_t))) = 0x100;

    // run
    ll_eval(context, 0x1000-sizeof(vword_t));

    delete context;

    return 0;
}

