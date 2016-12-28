#include <string.h>
#include <stdlib.h>

#include <giecs/logger.h>
#include <giecs/context.h>

#include <giecs/ll.h>

#include <giecs/lisp/ll.h>
#include <giecs/lisp/parser.h>

Logger* lisp_logger;
Logger* lisp_parser_logger;
Logger* lisp_atom_logger;

extern Namespace* default_namespace;
/*
vword_t lisp_exec(Context* context, const char* str)
{
    vword_t stack = 4096*0x1000 - VWORD_SIZE;
    vword_t entry_point = 0x2000;

    SNode* ast = new SNode(LIST, (char*)str);
    lisp_parse(context, entry_point, ast);

    // set entry point and run
    context->write_word(stack, entry_point);
    ll_eval(stack);
}
*/
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
        SNode* sn = new SNode(context, *p);
        *p += sn->vmem_size();

        // parse it on stack
        size_t n = 0;
        vword_t pushs, fn;

        StackFrame f = StackFrame(context, 0);

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
                f = StackFrame(context, pt);
                ll_eval(f);
                pt = f.ptr();
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

                f = StackFrame(context, pt);
                ll_resw(f);
                pt = f.ptr();
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

void ll_function(StackFrame& stack)
{
    SNode* plist = new SNode(stack);
    SNode* val = new SNode(stack);

    plist->dump();
    val->dump();

    int nparam = plist->subnodes->numOfElements();
    size_t reqb = nparam * VWORD_SIZE;

    vword_t p = stack.ptr();
    vword_t pt = eval_params(stack.context, &p, reqb);
    stack = StackFrame(stack.context, p);
    StackFrame f = StackFrame(stack.context, pt);

    // use stack pointer as group id for symbols
    Namespace* old_ns = default_namespace;
    default_namespace = new Namespace(default_namespace);

    // bind parameters
    ListIterator<SNode*> it = ListIterator<SNode*>(plist->subnodes);
    int i = 0;
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();
        vword_t start = f.ptr() + (i++)*VWORD_SIZE;
        add_symbol(sn->string, start);

        it.next();
    }

    size_t n = f.ptr();
    f.move(lisp_parse_size(val));
    lisp_parse(f.context, f.ptr(), val);
    f.move(VWORD_SIZE);
    ll_eval(f);
    n -= f.ptr();

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
    stack.move(n);
    stack.context->copy(stack.ptr(), f.ptr(), n);
}

void ll_expand_macro(StackFrame& stack)
{
    SNode* plist = new SNode(stack);
    SNode* val = new SNode(stack);

    //plist->dump();
    //val->dump();

    if(plist->type != LIST)
    {
        printf("error: no parameterlist\n");
        return;
    }

    int pnum = plist->subnodes->numOfElements();
    SNode** params = (SNode**) malloc(pnum * sizeof(SNode*));
    char** names = (char**) malloc(pnum * sizeof(char*));

    int i = 0;
    ListIterator<SNode*> it = ListIterator<SNode*>(plist->subnodes);
    while(! it.isLast())
    {
        SNode* c = it.getCurrent();
        if(c->type == SYMBOL)
        {
            names[i] = c->string;
            params[i] = new SNode(stack);
            i++;
        }
        else
        {
            printf("error: only symbols in paramlist allowed\n");
        }

        it.next();
    }

    val->replace(names, params, pnum);

    stack.move(lisp_parse_size(val));
    lisp_parse(stack.context, stack.ptr(), val);
}

void ll_macro(StackFrame& stack)
{
    ll_expand_macro(stack);

    stack.move(-VWORD_SIZE);
    ll_eval(stack);
}

void ll_eval_param(StackFrame& stack)
{
    vword_t fn = stack.pop_word();
    vword_t l = stack.pop_word();

    vword_t p = stack.ptr();
    vword_t pt = eval_params(stack.context, &p, l);

    stack = StackFrame(stack.context, p);
    StackFrame f = StackFrame(stack.context, pt);

    f.push_word(fn);

    // eval
    size_t n = f.ptr() + l + VWORD_SIZE;

    ll_eval(f);
    n -= f.ptr();

    // copy back
    stack.move(n);
    stack.context->copy(stack.ptr(), f.ptr(), n);
}

void ll_isdef(StackFrame& stack)
{
    SNode* sym = new SNode(stack);

    if(resolve_symbol(sym->string) == NULL)
        stack.push_byte((vbyte_t) 0);
    else
        stack.push_byte((vbyte_t) 1);
}

void ll_declare(StackFrame& stack)
{
    static Logger* logger = new Logger(lisp_logger, "declare");

    static vword_t def_start = 0x2000;

    SNode* parent = new SNode(stack);
    SNode* name = new SNode(stack);
    SNode* value = new SNode(stack);

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

            vword_t tmp = def_start;
            size_t len = lisp_parse(stack.context, def_start, value);

            if(value->type == LIST)
            {
                //expand(context, def_start, &tmp, false, false);

                len = stack.ptr();
                stack.push_word(def_start);
                ll_eval(stack);
                len -= stack.ptr();

                stack.context->copy(def_start, stack.ptr(), len);
                stack.move(-len);
            }

            def_start += len;

//            logger->log(linfo, "declared \'%s\': 0x%x, %d bytes", name->string, def_start, len);
        }
        else
            logger->log(lerror, "symbol \'%s\' already in use", name->string);
    }
    else
        logger->log(lerror, "name argument must be a symbol!");

    delete name;
    delete value;
}

void ll_asm(StackFrame& stack)
{
    SNode* ast = new SNode(stack);
    size_t l = asm_parse(stack.context, 0x2000, ast);
    stack.push_word(0x2000);
}

void ll_quote(StackFrame& stack)
{
    SNode* ast = new SNode(stack);

    if(ast->type == LIST && quote_stack != (vword_t)0)
    {
        quote_stack -= lisp_parse_size(ast);
        lisp_parse(stack.context, quote_stack, ast);
//        quote_stack = ll_expand(context, quote_stack);

        stack.push_word(quote_stack);
    }
    else
    {
        stack.move(lisp_parse_size(ast));
        lisp_parse(stack.context, stack.ptr(), ast);
    }
}

void ll_lmap(StackFrame& stack)
{
    SNode* fn = new SNode(stack);
    SNode* list = new SNode(stack);

    ListIterator<SNode*> it = ListIterator<SNode*>(list->subnodes);
    while(! it.isLast())
    {
        SNode* param = it.getCurrent();

        stack.move(param->vmem_size());
        param->write_vmem(stack.context, stack.ptr());

        stack.move(lisp_parse_size(fn));
        lisp_parse(stack.context, stack.ptr(), fn);
        if(fn->type == LIST)
            stack.move(-VWORD_SIZE);

        ll_eval(stack);

        it.next();
    }
}

void ll_progn(StackFrame& stack)
{
    SNode* list = new SNode(stack);

    ListIterator<SNode*> it = ListIterator<SNode*>(list->subnodes);
    while(! it.isLast())
    {
        SNode* fn = it.getCurrent();

        stack.move(lisp_parse_size(fn));
        lisp_parse(stack.context, stack.ptr(), fn);

        if(fn->type == LIST)
            stack.move(-VWORD_SIZE);

        ll_eval(stack);

        it.next();
    }
}

