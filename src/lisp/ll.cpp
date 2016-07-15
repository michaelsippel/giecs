#include <string.h>
#include <stdlib.h>

#include <logger.h>
#include <context.h>

#include <ll.h>

#include <lisp/ll.h>
#include <lisp/parser.h>

Logger* lisp_logger;
Logger* lisp_parser_logger;
Logger* lisp_atom_logger;

extern Namespace* default_namespace;

vword_t lisp_exec(Context* context, const char* str)
{
    vword_t stack = 4096*0x1000 - VWORD_SIZE;
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

    default_namespace = new Namespace(NULL);
    add_symbol((char*)NULL, 0, 0, default_namespace);

    // system functions
    add_symbol("eval", context->add_ll_fn(ll_eval), VWORD_SIZE);
    add_symbol("deval", context->add_ll_fn(ll_deval), 2*VWORD_SIZE);
    add_symbol("nop", context->add_ll_fn(ll_nop));

    add_symbol("setrelative", context->add_ll_fn(ll_setrelative));
    add_symbol("pop", context->add_ll_fn(ll_pop));

    add_symbol("evalparam", context->add_ll_fn(ll_eval_param), 2*VWORD_SIZE);
    add_symbol("syscall", context->add_ll_fn(ll_syscall), 6*VWORD_SIZE);

    add_symbol("load", context->add_ll_fn(ll_load), VWORD_SIZE);

    add_symbol("oif", context->add_ll_fn(ll_cond), 2*VWORD_SIZE+1);
    add_symbol("eqw", context->add_ll_fn(ll_eqw), 2*VWORD_SIZE);
    add_symbol("eqb", context->add_ll_fn(ll_eqb), 2);

    add_symbol("resw", context->add_ll_fn(ll_resw), VWORD_SIZE);
    add_symbol("setw", context->add_ll_fn(ll_setw), 2*VWORD_SIZE);
    add_symbol("resb", context->add_ll_fn(ll_resb), VWORD_SIZE);
    add_symbol("setb", context->add_ll_fn(ll_setb), VWORD_SIZE+1);
    add_symbol("map", context->add_ll_fn(ll_map), 4*VWORD_SIZE);

    add_symbol("printi", context->add_ll_fn(ll_printi), VWORD_SIZE);
    add_symbol("printb", context->add_ll_fn(ll_printb), 1);
    add_symbol("+", context->add_ll_fn(ll_addi), 2*VWORD_SIZE);
    add_symbol("-", context->add_ll_fn(ll_subi), 2*VWORD_SIZE);
    add_symbol("*", context->add_ll_fn(ll_muli), 2*VWORD_SIZE);
    add_symbol("/", context->add_ll_fn(ll_divi), 2*VWORD_SIZE);

    // lisp macro
    add_symbol("expand", context->add_ll_fn(ll_expand));
    add_symbol("quote", context->add_ll_fn(ll_quote));
    add_symbol("asm", context->add_ll_fn(ll_asm));
    add_symbol("declare", context->add_ll_fn(ll_declare));
    add_symbol("isdecl", context->add_ll_fn(ll_isdef));
    add_symbol("function", context->add_ll_fn(ll_function));
    add_symbol("macro", context->add_ll_fn(ll_macro));
    add_symbol("lmap", context->add_ll_fn(ll_lmap));
    add_symbol("progn", context->add_ll_fn(ll_progn));
}

static vword_t quote_stack = 0;

vword_t eval_params(Context* context, vword_t* p, size_t l)
{
    vword_t pt = *p; // working pointer (temporal pushs)
    vbyte_t* buf = (vbyte_t*) malloc(l * sizeof(vbyte_t));
    vbyte_t* dest = buf;

    size_t i = 0;
    while(i < l)
    {
        quote_stack = *p;

        // get snode
        SNode* sn = new SNode(LIST);
        *p += sn->read_vmem(context, *p);

        // parse it on stack
        size_t n = 0;
        vword_t pushs, fn;

        // self-eval lists
        switch(sn->type)
        {
            case INTEGER:
                pt -= lisp_parse_size(sn);
                n = lisp_parse(context, pt, sn);
                break;

            case LIST:
                pt -= lisp_parse_size(sn);
                n = lisp_parse(context, pt, sn);

                pushs = context->read_word(pt);
                pt += VWORD_SIZE;

                fn = context->read_word(pt);
                if(fn != resolve_symbol("quote")->start)
                    quote_stack = 0;

                n = pt + pushs;
                pt = ll_eval(context, pt);
                n -= pt;
                break;

            case STRING:
                pt -= VWORD_SIZE;
                context->write_word(pt, quote_stack);
                n = VWORD_SIZE;

                quote_stack += lisp_parse(context, quote_stack, sn);
                break;

            case SYMBOL:
                pt -= lisp_parse_size(sn);
                n = lisp_parse(context, pt, sn);

                pt = ll_resw(context, pt);
                break;
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

    return pt;
}

vword_t ll_function(Context* context, vword_t p)
{
    SNode* plist = new SNode(LIST);
    p += plist->read_vmem(context, p);

    SNode* val = new SNode(LIST);
    p += val->read_vmem(context, p);

//    plist->dump();
//    val->dump();

    int nparam = plist->subnodes->numOfElements();
    size_t reqb = nparam * VWORD_SIZE;

    vword_t pt = eval_params(context, &p, reqb);

    // use stack pointer as group id for symbols
    Namespace* old_ns = default_namespace;
    default_namespace = new Namespace(default_namespace);

    // bind parameters
    ListIterator<SNode*> it = ListIterator<SNode*>(plist->subnodes);
    int i = 0;
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();
        vword_t start = pt + (i++)*VWORD_SIZE;
        add_symbol(sn->string, start, 0);

        it.next();
    }

    size_t n = pt;
    pt -= lisp_parse_size(val);
    lisp_parse(context, pt, val);

    pt = ll_eval(context, pt+VWORD_SIZE);

    n -= pt;

    // unbind parameters
    it.setFirst();
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();
        remove_symbol(sn->string);
        it.next();
    }

    delete default_namespace;
    default_namespace = old_ns;

    // copy back
    p -= n;
    context->copy(p, pt, n);

    return p;
}

vword_t ll_expand_macro(Context* context, vword_t p)
{
    SNode* plist = new SNode(LIST);
    p += plist->read_vmem(context, p);

    SNode* val = new SNode(LIST);
    p += val->read_vmem(context, p);

    p = expand_macro(context, p, plist, val, &p);

    return p;
}

vword_t ll_macro(Context* context, vword_t p)
{
    p = ll_expand_macro(context, p);
    p = ll_eval(context, p+VWORD_SIZE);

    return p;
}

vword_t ll_eval_param(Context* context, vword_t p)
{
    vword_t fn = context->read_word(p);
    p += VWORD_SIZE;
    vword_t l = context->read_word(p);
    p += VWORD_SIZE;

    vword_t pt = eval_params(context, &p, l);

    pt -= VWORD_SIZE;
    context->write_word(pt, fn);

    // eval
    size_t n = pt + l + VWORD_SIZE;
    pt = ll_eval(context, pt);
    n -= pt;

    // copy back
    p -= n;
    context->copy(p, pt, n);

    return p;
}

vword_t ll_isdef(Context* context, vword_t p)
{
    SNode* sym = new SNode(context, p);
    p += sym->vmem_size();

    vbyte_t t = (vbyte_t)1;
    vbyte_t f = (vbyte_t)0;

    p -= 1;
    if(resolve_symbol(sym->string) == NULL)
        context->write(p, 1, &f);
    else
        context->write(p, 1, &t);

    return p;
}

vword_t ll_declare(Context* context, vword_t p)
{
    static Logger* logger = new Logger(lisp_logger, "declare");

    static vword_t def_start = 0xA0000;

    SNode* parent = new SNode(context, p);
    p += parent->vmem_size();
    SNode* name = new SNode(context, p);
    p += name->vmem_size();
    SNode* value = new SNode(context, p);
    p += value->vmem_size();

    if(name->type == SYMBOL)
    {
        if(resolve_symbol(name->string) == NULL)
        {
            Namespace* ns;

            vword_t parent_id = parent->integer;
            if(parent_id == -1)
                ns = default_namespace;
            else
                ns = resolve_symbol(parent_id)->ns;

            ns->add_symbol(name->string, def_start);

            size_t len = p;
            size_t pl = lisp_parse_size(value);
            p -= pl;
            lisp_parse(context, p, value);
/*
            size_t reqb = 0;
            if(value->type == LIST)
            {
                // TODO: clean this up
				vword_t p2 = p;
                p2 = expand(context, p2p, &p2, false, false);
                p2 = ll_eval(context, p2+VWORD_SIZE);

                vword_t ptr = context->read_word(p2+VWORD_SIZE);
                reqb = get_reqb(ptr);
            }

            ns->add_parsepoint(def_start, reqb);
*/
            if(value->type == LIST)
            {
              //  p = ll_expand(context, p);
                p = ll_eval(context, p+VWORD_SIZE);
            }

            len -= p;

            context->copy(def_start, p, len);
            def_start += len;
            p += len;

            logger->log(linfo, "declared \'%s\': 0x%x, %d bytes", name->string, def_start, len);
        }
        else
            logger->log(lerror, "symbol \'%s\' already in use", name->string);
    }
    else
        logger->log(lerror, "name argument must be a symbol!");

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
    SNode* ast = new SNode(context, p);
    p += ast->vmem_size();

    if(ast->type == LIST && quote_stack != (vword_t)0)
    {
        quote_stack -= lisp_parse_size(ast);
        lisp_parse(context, quote_stack, ast);
        quote_stack = ll_expand(context, quote_stack);

        p -= VWORD_SIZE;
        context->write_word(p, quote_stack);
    }
    else
    {
        p -= lisp_parse_size(ast);
        lisp_parse(context, p, ast);
    }

    return p;
}

vword_t ll_lmap(Context* context, vword_t p)
{
    SNode* fn = new SNode(context, p);
    p += fn->vmem_size();

    SNode* list = new SNode(context, p);
    p += list->vmem_size();

    ListIterator<SNode*> it = ListIterator<SNode*>(list->subnodes);
    while(! it.isLast())
    {
        SNode* param = it.getCurrent();

        p -= param->vmem_size();
        param->write_vmem(context, p);

        p -= lisp_parse_size(fn);
        lisp_parse(context, p, fn);
        if(fn->type == LIST)
            p += VWORD_SIZE;
        p = ll_eval(context, p);

        it.next();
    }

    return p;
}

vword_t ll_progn(Context* context, vword_t p)
{
    SNode* list = new SNode(context, p);
    p += list->vmem_size();

    ListIterator<SNode*> it = ListIterator<SNode*>(list->subnodes);
    while(! it.isLast())
    {
        SNode* fn = it.getCurrent();

        p -= lisp_parse_size(fn);
        lisp_parse(context, p, fn);

        if(fn->type == LIST)
            p += VWORD_SIZE;
        p = ll_eval(context, p);

        it.next();
    }

    return p;
}

