
#include <context.h>
#include <ll.h>
#include <lisp/ll.h>
#include <lisp/parser.h>

extern Namespace* default_namespace;

size_t expand_evalparam(Context* context, vword_t p, vword_t fn, List<SNode*>* plist)
{
    size_t len = 4*VWORD_SIZE;

    int j = plist->numOfElements() + 1;

    vbyte_t* listbuf = (vbyte_t*) malloc(2*sizeof(vword_t)*j);
    vword_t* lp = (vword_t*) listbuf;

    *lp++ = VWORD_SIZE;
    *lp++ = fn;

    vword_t resbuf[3];
    resbuf[0] = 2*VWORD_SIZE;
    resbuf[1] = resolve_symbol("resw")->start;

    vword_t ptr, tmp;

    ListIterator<SNode*> it = ListIterator<SNode*>(plist);
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();
        switch(sn->type)
        {
            case INTEGER:
                *lp++ = VWORD_SIZE;
                *lp++ = sn->integer;
                break;

            case SYMBOL:
                ptr = p+len;
                len += context->write(ptr, 2*VWORD_SIZE, (vbyte_t*) &resbuf);
                len += asm_parse(context, p+len, sn);

                *lp++ = -1;
                *lp++ = ptr;
                break;

            case STRING:
                ptr = p+len;
                lisp_parse(context, ptr, sn);
                *lp++ = VWORD_SIZE;
                *lp++ = ptr;
                break;

            case LIST:
                ptr = p+len;
                tmp = ptr;
                len += lisp_parse(context, ptr, sn);
                len += expand(context, ptr, &tmp, false, true);

                *lp++ = -1;
                *lp++ = ptr;
                break;
        }

        it.next();
    }

    ptr = p+len;
    len += context->write(ptr, 2*VWORD_SIZE*j, listbuf);
    free(listbuf);

    vword_t b0[4];
    b0[0] = 3*VWORD_SIZE;
    b0[1] = resolve_symbol("deval")->start;
    b0[2] = j;
    b0[3] = ptr;

    context->write(p, 4*VWORD_SIZE, (vbyte_t*) &b0);

    return len;
}

vword_t expand_macro(Context* context, vword_t pt, SNode* plist, SNode* val, vword_t* p)
{
    if(plist->type != LIST)
    {
        printf("error: no parameterlist\n");
        return pt;
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
            params[i] = new SNode(context, *p);
            *p += params[i]->vmem_size();

            i++;
        }
        else
        {
            printf("error: only symbols in paramlist allowed\n");
        }

        it.next();
    }

    val->replace(names, params, pnum);
    val->dump();

    pt -= lisp_parse_size(val);
    lisp_parse(context, pt, val);

    return pt;
}

/*
vword_t ll_expand_function(Context* context, vword_t p)
{
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
        vword_t start = - (i++)*VWORD_SIZE;
        add_symbol(sn->string, start, 0);

        it.next();
    }

	val->dump();

    p -= lisp_parse_size(val);
    lisp_parse(context, p, val);
	printf("expand function first\n");
    vword_t fn = expand(context, p, false, false);
	context->dump(fn, 8);

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

    pt -= 5*VWORD_SIZE;

    vword_t srb[4];
    srb[0] = 3*VWORD_SIZE;
	srb[1] = resolve_symbol("pop")->start;
	srb[2] = reqb;
    srb[3] = resolve_symbol("setrelative")->start;

    context->write(pt, 4*VWORD_SIZE, (vbyte_t*) &srb);
    add_parsepoint(pt, reqb);

    p -= VWORD_SIZE;
    context->write_word(p, pt);
    p -= VWORD_SIZE;
    context->write_word(p, VWORD_SIZE);

    return p;
}
*/
// compile to lower level to avoid reparsing
static vword_t pt = 0x10000;
vword_t ll_expand(Context* context, vword_t p)
{
    size_t l = expand(context, pt, &p, false, false);

    p -= VWORD_SIZE;
    context->write_word(p, pt);
    p -= VWORD_SIZE;
    context->write_word(p, VWORD_SIZE);

    pt += l;

    return p;
}

size_t expand(Context* context, vword_t pt, vword_t* p, bool quoted, bool quoteptr)
{
    size_t len = 0;

    vword_t flen = context->read_word(*p);
    *p += VWORD_SIZE;
    vword_t ptr = context->read_word(*p);
    *p += VWORD_SIZE;

    if(ptr == resolve_symbol("evalparam")->start)
    {
        vword_t fn = context->read_word(*p);
        *p += VWORD_SIZE;
        vword_t l = context->read_word(*p);
        *p += VWORD_SIZE;

        List<SNode*>* plist = new List<SNode*>();
        size_t n = 0;

        // collect params
        while(n < l)
        {
            SNode* sn = new SNode(context, *p);
            *p += sn->vmem_size();
            plist->pushBack(sn);

            // speculated size after execution
            n += VWORD_SIZE; // TODO
        }

        len = expand_evalparam(context, pt, fn, plist);

        delete plist;
    }
    /*    else if(ptr == resolve_symbol("macro")->start && !quoted)
        {
            SNode* plist = new SNode(LIST);
            *p += plist->read_vmem(context, *p);

            SNode* val = new SNode(LIST);
            *p += val->read_vmem(context, *p);

            printf("now xpand THE MARCO\n");
            pt = expand_macro(context, pt, plist, val, p);
        }
            else if(ptr == resolve_symbol("function")->start)
            {
                p = ll_expand_function(context, p);
            }
        /*    else if(ptr == resolve_symbol("quote")->start)
            {
                SNode* ast = new SNode(context, p);
                p += ast->vmem_size();

                vword_t len;

                if(ast->type == LIST && quoteptr)
                {
                    pt -= lisp_parse_size(ast);
                    lisp_parse(context, pt, ast);

                    vword_t fn = expand(context, pt, true, false);

                    p -= VWORD_SIZE;
                    context->write_word(p, fn);

                    len = VWORD_SIZE;
                }
                else
                {
                    p -= lisp_parse_size(ast);
                    lisp_parse(context, p, ast);

                    if(ast->type == LIST)
                    {
                        p = expand(context, p, true, false);
                        len = context->read_word(p);
                    }
                    else
                        len = VWORD_SIZE;
                }

                p -= VWORD_SIZE;
                context->write_word(p, resolve_symbol("nop")->start);
                p -= VWORD_SIZE;
                context->write_word(p, len+VWORD_SIZE);

        //		context->dump(p, len/VWORD_SIZE+1);
            }
            else if(reqb > 0)
            {
                p = expand_evalparam(context, p, ptr, reqb);
            }
        */
    else
    {
        // don't expand lowlevel functions
        vword_t l = context->read_word(ptr);
        if(l == -1)
        {
            *p -= 2*VWORD_SIZE;
            len = flen+VWORD_SIZE;
            context->copy(pt, *p, len);
            *p += len;

            return len;
        }
    }
    /*
    	len -= pt;

        if(!quoted)
        {
            return len + expand(context, pt, &pt, false, false);
        }
    */
    return len;
}

