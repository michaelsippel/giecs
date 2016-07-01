
#include <context.h>
#include <ll.h>
#include <lisp/ll.h>
#include <lisp/parser.h>

extern Namespace* default_namespace;

static vword_t pt = 0x80000; // TODO

vword_t expand_evalparam(Context* context, vword_t p, vword_t fn, size_t l)
{
    List<SNode*>* plist = new List<SNode*>();
    size_t n = 0;
    int j = 1;

    // collect params
    while(n < l)
    {
        SNode* sn = new SNode(context, p);
        p += sn->vmem_size();
        plist->pushBack(sn);

        // speculated size after execution
        n += VWORD_SIZE; // TODO
        j++;
    }

    vbyte_t* listbuf = (vbyte_t*) malloc(2*sizeof(vword_t)*j);
    vword_t* lp = (vword_t*) listbuf;

    *lp++ = VWORD_SIZE;
    *lp++ = fn;

    ListIterator<SNode*> it = ListIterator<SNode*>(plist);
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();
        //sn->dump();

        vword_t resbuf[3];
        resbuf[0] = 2*VWORD_SIZE;
        resbuf[1] = resolve_symbol("resw")->start;
        size_t l;

        switch(sn->type)
        {
            case INTEGER:
                *lp++ = VWORD_SIZE;
                *lp++ = sn->integer;
                break;

            case SYMBOL:
                pt -= VWORD_SIZE;
                asm_parse(context, pt, sn);

                pt -= 2*VWORD_SIZE;
                context->write(pt, 2*VWORD_SIZE, (vbyte_t*) &resbuf);

                *lp++ = -1;
                *lp++ = pt;
                break;

            case STRING:
                pt -= lisp_parse_size(sn);
                lisp_parse(context, pt, sn);

                *lp++ = VWORD_SIZE;
                *lp++ = pt;
                break;

            case LIST:
                l = p;
                p -= lisp_parse_size(sn);
                lisp_parse(context, p, sn);
                p = ll_expand(context, p);
                l -= p;

                pt -= l;
                context->copy(pt, p, l);
                p += l;

                *lp++ = -1;
                *lp++ = pt;
                break;
        }

        it.next();
    }

    pt -= 2*VWORD_SIZE*j;
    context->write(pt, 2*VWORD_SIZE*j, listbuf);

    free(listbuf);
    delete plist;

    vword_t b0[4];
    b0[0] = 3*VWORD_SIZE;
    b0[1] = resolve_symbol("deval")->start;
    b0[2] = j;
    b0[3] = pt;

    p -= 4*VWORD_SIZE;
    context->write(p, 4*VWORD_SIZE, (vbyte_t*) &b0);

    size_t reqb = get_reqb(fn);
    add_parsepoint(p, reqb-l);

    return p;
}

vword_t ll_expand_function(Context* context, vword_t p)
{
    printf("expand funciton\n");

    SNode* plist = new SNode(LIST);
    p += plist->read_vmem(context, p);

    SNode* val = new SNode(LIST);
    p += val->read_vmem(context, p);

    //plist->dump();
    //val->dump();

    int nparam = plist->subnodes->numOfElements();
    size_t reqb = nparam * VWORD_SIZE;

    Namespace* old_ns = default_namespace;
    default_namespace = new Namespace(default_namespace);

    // bind parameters
    ListIterator<SNode*> it = ListIterator<SNode*>(plist->subnodes);
    int i = 0;
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();
        vword_t start = - (1+i++)*VWORD_SIZE;
        add_symbol(sn->string, start, 0);

        it.next();
    }

    pt -= lisp_parse_size(val);
    lisp_parse(context, pt, val);
    vword_t fn = ll_expand(context, pt);

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

    pt -= 3*VWORD_SIZE;

    vword_t srb[3];
    srb[0] = 2*VWORD_SIZE;
    srb[1] = resolve_symbol("setrelative")->start;
    srb[2] = fn;

    context->write(pt, 3*VWORD_SIZE, (vbyte_t*) &srb);
    add_parsepoint(pt, reqb);

    p -= VWORD_SIZE;
    context->write_word(p, pt);
    p -= VWORD_SIZE;
    context->write_word(p, VWORD_SIZE);

    return p;
}

// compile to lower level to avoid reparsing
vword_t ll_expand(Context* context, vword_t p)
{
    vword_t op = p;

    p += VWORD_SIZE;
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;

    size_t reqb = get_reqb(ptr);

    if(ptr == resolve_symbol("evalparam")->start)
    {
        vword_t fn = context->read_word(p);
        p += VWORD_SIZE;
        vword_t l = context->read_word(p);
        p += VWORD_SIZE;

        p = expand_evalparam(context, p, fn, l);
        //context->dump(p, 8);
    }
    else if(ptr == resolve_symbol("macro")->start)
    {
        p = ll_expand_macro(context, p);
        p = ll_expand(context, p);
    }
    else if(ptr == resolve_symbol("function")->start)
    {
        p = ll_expand_function(context, p);
    }
    else if(ptr == resolve_symbol("quote")->start)
    {
        p = ll_quote(context, p);

        vword_t len = context->read_word(p);
        p -= VWORD_SIZE;
        context->write_word(p, resolve_symbol("nop")->start);
        p -= VWORD_SIZE;
        context->write_word(p, len+VWORD_SIZE);
    }
    else if(reqb > 0)
    {
        p = expand_evalparam(context, p, ptr, reqb);
    }
    else
    {
        p = op;
    }

    return p;
}

