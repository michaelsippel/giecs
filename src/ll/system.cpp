#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <ll.h>

int ll_exit_(void* p)
{
    vword_t val = *((vword_t*) p);

    printf("Exit with value %d\n", val);
    exit(val);

    return (int)(-sizeof(vword_t));
}

int ll_addi_(void* p)
{
    vword_t* stack = (vword_t*) p;
    vword_t a = *stack++;
    vword_t b = *stack++;

    *--stack = a + b;

    return (int)(-sizeof(vword_t));
}

int ll_printi_(void* p)
{
    printf("%d\n", ((vword_t*)p)[0]);
    return (int)(-sizeof(vword_t));
}

int ll_printb_(void* p)
{
    printf("%s\n", ((vbyte_t*)p)[0] ? "true" : "false");
    return (int)(-sizeof(vbyte_t));
}

int ll_nop_(void* p)
{
    return 0;
}

LL_STUB(ll_nop, ll_nop_);
LL_STUB(ll_exit, ll_exit_);
LL_STUB(ll_addi, ll_addi_);
LL_STUB(ll_printi, ll_printi_);
LL_STUB(ll_printb, ll_printb_);


