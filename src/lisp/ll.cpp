#include <context.h>

#include <ll.h>
#include <lisp/ll.h>
#include <lisp/parser.h>

void init_lisp(Context* context)
{
    // system functions
    add_symbol("eval", context->add_ll_fn(ll_eval));
    add_symbol("deval", context->add_ll_fn(ll_deval));
    add_symbol("nop", context->add_ll_fn(ll_nop));

    add_symbol("if", context->add_ll_fn(ll_cond));
    add_symbol("eq", context->add_ll_fn(ll_eq));

    add_symbol("map", context->add_ll_fn(ll_map));
    add_symbol("exit", context->add_ll_fn(ll_exit));
    add_symbol("printi", context->add_ll_fn(ll_printi));
    add_symbol("printb", context->add_ll_fn(ll_printb));
    add_symbol("addi", context->add_ll_fn(ll_addi));


    // lisp macro
    add_symbol("declare", context->add_ll_fn(ll_declare));
}

vword_t ll_declare(Context* context, vword_t p)
{
    printf("Lisp declare\n");

    return p;
}

