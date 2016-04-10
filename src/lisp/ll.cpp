#include <logger.h>
#include <context.h>

#include <ll.h>

#include <lisp/ll.h>
#include <lisp/parser.h>

Logger* lisp_logger;
Logger* lisp_parser_logger;
Logger* lisp_atom_logger;

vword_t lisp_exec(Context* context, const char* str)
{
    vword_t stack = 4096*0x100 - VWORD_SIZE;
    vword_t entry_point = 0x2000;

    SNode* ast = new SNode(LIST, (char*)str);
    lisp_parse(context, entry_point, ast);

    // set entry point and run
    context->write_word(stack, entry_point);
    return ll_eval(context, stack);
}

void init_lisp(Context* context)
{
    lisp_logger = new Logger("lisp");
    lisp_parser_logger = new Logger(lisp_logger, "parser");
    lisp_atom_logger = new Logger(lisp_parser_logger, "atom");

    // system functions
    add_symbol("eval", context->add_ll_fn(ll_eval), VWORD_SIZE);
    add_symbol("deval", context->add_ll_fn(ll_deval), 2*VWORD_SIZE);
    add_symbol("nop", context->add_ll_fn(ll_nop), -1);

    add_symbol("genfn", context->add_ll_fn(ll_gen_fn), 2*VWORD_SIZE);

    add_symbol("if", context->add_ll_fn(ll_cond), 2*VWORD_SIZE+1);
    add_symbol("eq", context->add_ll_fn(ll_eq), 2*VWORD_SIZE);

    add_symbol("resw", context->add_ll_fn(ll_resw), VWORD_SIZE);
    add_symbol("setw", context->add_ll_fn(ll_setw), 2*VWORD_SIZE);
    add_symbol("map", context->add_ll_fn(ll_map), 4*VWORD_SIZE);
    add_symbol("exit", context->add_ll_fn(ll_exit), VWORD_SIZE);
    add_symbol("printi", context->add_ll_fn(ll_printi), VWORD_SIZE);
    add_symbol("printb", context->add_ll_fn(ll_printb), 1);
    add_symbol("+", context->add_ll_fn(ll_addi), 2*VWORD_SIZE);

    // lisp macro
    add_symbol("quote", context->add_ll_fn(ll_quote));
    add_symbol("asm", context->add_ll_fn(ll_asm));
    add_symbol("declare", context->add_ll_fn(ll_declare));
}

static vword_t quote_stack = 0x0;

vword_t ll_gen_fn(Context* context, vword_t p)
{
    vword_t fn = context->read_word(p);
    p += VWORD_SIZE;
    vword_t l = context->read_word(p);
    p += VWORD_SIZE;

    vword_t pt = p; // working pointer (temporal pushs)
    vbyte_t* buf = (vbyte_t*) malloc(l * sizeof(vbyte_t));
    vbyte_t* dest = buf;

    size_t i = 0;
    while(i < l && p < 0x100000)
    {
        quote_stack = p;

        // get snode
        SNode* sn = new SNode(LIST);
        p += sn->read_vmem(context, p);

        // parse it on stack
        pt -= lisp_parse_size(sn);
        size_t n = lisp_parse(context, pt, sn);

        // self-eval lists
        if(sn->type == LIST)
        {
            vword_t pushs = context->read_word(pt);
            pt += VWORD_SIZE;

            vword_t fn = context->read_word(pt);
            if(fn != resolve_symbol("quote").start)
                quote_stack = 0;

            n = pt + pushs;
            pt = ll_eval(context, pt);
            n -= pt;
        }

        if((i+n) > l)
        {
            printf("getting more than needed (%d). cutting down to %d bytes\n", n, l-i);
            n = l-i;
        }

        dest += context->read(pt, n, dest);
        pt += n;
        i += n;
    }

    quote_stack = 0;

    pt -= i;
    context->write(pt, i, buf);

    free(buf);

    pt -= VWORD_SIZE;
    context->write_word(pt, fn);

    // copy back
    size_t n = pt + i + VWORD_SIZE;
    pt = ll_eval(context, pt);
    n -= pt;

    buf = (vbyte_t*) malloc(n * sizeof(vbyte_t));
    context->read(pt, n, buf);

    p -= n;
    context->write(p, n, buf);
    return p;
}

vword_t ll_declare(Context* context, vword_t p)
{
    static Logger* logger = new Logger(lisp_logger, "declare");

    static vword_t def_top = 0xA0000;

    SNode* name = new SNode(context, p);
    p += name->vmem_size();
    SNode* value = new SNode(context, p);
    p += value->vmem_size();

    if(name->type == SYMBOL)
    {
        if(resolve_symbol(name->string).start == (vword_t)0)
        {
            size_t len = def_top;
            def_top -= lisp_parse_size(value);
            lisp_parse(context, def_top, value);

            // execute lists
            if(value->type == LIST)
                def_top = ll_eval(context, def_top+VWORD_SIZE);

            len -= def_top;

            logger->log(linfo, "declared \'%s\': 0x%x, %d bytes", name->string, def_top, len);

            add_symbol(name->string, def_top);
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
    SNode* ast = new SNode(context, p);
    p += ast->vmem_size();

    size_t l = asm_parse(context, 0x2000, ast);

    p -= VWORD_SIZE;
    context->write_word(p, 0x2000);

    return p;
}

vword_t ll_quote(Context* context, vword_t p)
{
    SNode* ast = new SNode(LIST);
    p += ast->read_vmem(context, p);

    if(ast->type == LIST && quote_stack != (vword_t)0)
    {
//		printf("quote with pointer!\n");
        p -= VWORD_SIZE;
        context->write_word(p, quote_stack);

        quote_stack += lisp_parse(context, quote_stack, ast);
    }
    else
    {
        p -= lisp_parse_size(ast);
        lisp_parse(context, p, ast);
    }

    return p;
}

