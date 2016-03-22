#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <reader.h>
#include <ll.h>

vword_t ll_exit(Context* context, vword_t p)
{
    vword_t val = context->read_word(p);

    printf("Exit with value %d\n", val);
    exit(val);

    return p + VWORD_SIZE;
}

vword_t ll_addi(Context* context, vword_t p)
{
    vword_t val[2];
    context->read(p, 2*VWORD_SIZE, (vbyte_t*) &val);

    vword_t ret = val[0] + val[1];
    context->write_word(p+VWORD_SIZE, ret);

    return p + VWORD_SIZE;
}

vword_t ll_printi(Context* context, vword_t p)
{
    vword_t val = context->read_word(p);
    printf("%d\n", val);

    return p + VWORD_SIZE;
}

vword_t ll_printb(Context* context, vword_t p)
{
    vbyte_t val;
    context->read(p, 1, &val);
    printf("%s\n", val ? "true" : "false");
    return p + 1;
}

vword_t ll_nop(Context* context, vword_t p)
{
    return p;
}

vword_t ll_ptest(Context* context, vword_t p)
{
    printf("Hello World from ptest!\n");

    vword_t w = context->read_word(p);
    p += VWORD_SIZE;

    SNode* ast = new SNode(context, w);
    ast->dump();

    return p;
}

