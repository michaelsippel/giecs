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
    add_symbol("EVAL", context->add_ll_fn(ll_eval));
    add_symbol("DEVAL", context->add_ll_fn(ll_deval));
    add_symbol("NOP", context->add_ll_fn(ll_nop));

    add_symbol("IF", context->add_ll_fn(ll_cond));
    add_symbol("EQ", context->add_ll_fn(ll_eq));

    add_symbol("RESW", context->add_ll_fn(ll_resw));
    add_symbol("SETW", context->add_ll_fn(ll_setw));
    add_symbol("MAP", context->add_ll_fn(ll_map));
    add_symbol("EXIT", context->add_ll_fn(ll_exit));
    add_symbol("PRINTI", context->add_ll_fn(ll_printi));
    add_symbol("PRINTB", context->add_ll_fn(ll_printb));
    add_symbol("ADDI", context->add_ll_fn(ll_addi));

    // lisp macro
    add_symbol("quote", context->add_ll_fn(ll_quote));
    add_symbol("asm", context->add_ll_fn(ll_asm));
    add_symbol("declare", context->add_ll_fn(ll_declare));
    add_symbol("genfn", context->add_ll_fn(ll_gen_fn));

    // lisp functions
    lisp_exec(context, "declare eval (quote (genfn EVAL 4))");

    lisp_exec(context, "declare if (quote (genfn IF 9))");
    lisp_exec(context, "declare eq (quote (genfn EQ 8))");

    lisp_exec(context, "declare resw (quote (genfn RESW 4))");
    lisp_exec(context, "declare setw (quote (genfn SETW 8))");
    lisp_exec(context, "declare map (quote (genfn MAP 16))");
    lisp_exec(context, "declare exit (quote (genfn EXIT 4))");
    lisp_exec(context, "declare printi (quote (genfn PRINTI 4))");
    lisp_exec(context, "declare printb (quote (genfn PRINTB 1))");
    lisp_exec(context, "declare + (quote (genfn ADDI 8))");

    lisp_exec(context, "declare brainfuck (quote (genfn BRAINFUCK 4))");

}

vword_t ll_gen_fn(Context* context, vword_t p)
{
    // get the two inputs
    SNode* a1 = new SNode(context, p); // function
    p += a1->vmem_size();
    SNode* a2 = new SNode(context, p); // needed length
    p += a2->vmem_size();

    if(a2->type != INTEGER)
    {
        printf("fn wrapper needs expected length\n");
        return p;
    }

    // collect applied arguments (ast) until the required length is reached
    size_t l = a2->integer;

    vword_t pt = p; // working pointer (temporal pushs)
    vbyte_t* buf = (vbyte_t*) malloc(l);
    vbyte_t* dest = buf;

    size_t i = 0;
    while(i < l)
    {
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

            n = pt + pushs;
            pt = ll_eval(context, pt);
            n -= pt;
        }

        dest += context->read(pt, n, dest);
        pt += n;
        i += n;
    }

    p -= l;
    context->write(p, l, buf);

    free(buf);

    p -= lisp_parse_size(a1);
    lisp_parse(context, p, a1);

    return ll_eval(context, p);
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
        if(resolve_symbol(name->string) == (vword_t)0)
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

    p -= lisp_parse_size(ast);
    lisp_parse(context, p, ast);

    return p;
}

