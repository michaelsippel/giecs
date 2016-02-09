#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <ll.h>

vword_t ll_exit(Context* context, vword_t p)
{
    vword_t val;
    context->read(p, sizeof(vword_t), (vbyte_t*) &val);

    printf("Exit with value %d\n", val);
    exit(val);

    return p+sizeof(vword_t);
}

vword_t ll_addi(Context* context, vword_t p)
{
    vword_t val[2];
    context->read(p, 2*sizeof(vword_t), (vbyte_t*) &val);

    vword_t ret = val[0] + val[1];
    context->write(p+sizeof(vword_t), sizeof(vword_t), (vbyte_t*) &ret);

    return p+sizeof(vword_t);
}

vword_t ll_printi(Context* context, vword_t p)
{
    vword_t val;
    context->read(p, sizeof(vword_t), (vbyte_t*) &val);

    printf("%d\n", val);
    return p+sizeof(vword_t);
}

vword_t ll_printb(Context* context, vword_t p)
{
    vbyte_t val;
    context->read(p, 1, &val);
    printf("%s\n", val ? "true" : "false");
    return p+1;
}

vword_t ll_nop(Context* context, vword_t p)
{
    return p;
}

