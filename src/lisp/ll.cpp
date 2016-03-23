#include <logger.h>
#include <context.h>

#include <ll.h>

#include <lisp/ll.h>
#include <lisp/parser.h>

Logger* lisp_logger;
Logger* lisp_parser_logger;
Logger* lisp_atom_logger;

void init_lisp(Context* context)
{
    lisp_logger = new Logger("lisp");
    lisp_parser_logger = new Logger(lisp_logger, "parser");
    lisp_atom_logger = new Logger(lisp_parser_logger, "atom");

    // system functions
    add_symbol("eval", context->add_ll_fn(ll_eval));
    add_symbol("deval", context->add_ll_fn(ll_deval));
    add_symbol("nop", context->add_ll_fn(ll_nop));

    add_symbol("if", context->add_ll_fn(ll_cond));
    add_symbol("eq", context->add_ll_fn(ll_eq));

    add_symbol("resw", context->add_ll_fn(ll_resw));
    add_symbol("map", context->add_ll_fn(ll_map));
    add_symbol("exit", context->add_ll_fn(ll_exit));
    add_symbol("printi", context->add_ll_fn(ll_printi));
    add_symbol("printb", context->add_ll_fn(ll_printb));
    add_symbol("addi", context->add_ll_fn(ll_addi));

    // lisp macro
    add_symbol("asm", context->add_ll_fn(ll_asm));
    add_symbol("declare", context->add_ll_fn(ll_declare));
}

vword_t ll_declare(Context* context, vword_t p)
{
    static Logger* logger = new Logger(lisp_logger, "declare");

    static vword_t base = 0x200;

    vword_t pn = context->read_word(p);
    p += VWORD_SIZE;
    vword_t vn = context->read_word(p);
    p += VWORD_SIZE;

    SNode* name = new SNode(context, pn);
    SNode* value = new SNode(context, vn);

    if(name->type == SYMBOL)
    {
        if(resolve_symbol(name->string) == (vword_t)0)
        {
            size_t len = lisp_parse(context, base, value);

            add_symbol(name->string, base);
            logger->log(linfo, "declared \'%s\': 0x%x", name->string, base);

            base += len;
        }
        else
            logger->log(lerror, "symbol \'%s\' already in use", name->string);
    }
    else
        logger->log(lerror, "first argument must be a symbol!");

    delete name;
    delete value;

    return p;
}

vword_t ll_asm(Context* context, vword_t p)
{
    vword_t n = context->read_word(p);

    SNode* ast = new SNode(context, n);
    size_t l = asm_parse(context, 0x2000, ast);

    context->write_word(p, 0x2000);
    p = ll_eval(context, p);

    return p;
}

