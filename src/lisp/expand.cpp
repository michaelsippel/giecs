
#include <context.h>
#include <ll.h>
#include <lisp/ll.h>
#include <lisp/parser.h>

// compile to lower level to avoid reparsing
vword_t ll_expand(Context* context, vword_t p)
{
    static vword_t pt = 0x10000; // TODO
    vword_t op = p;

    p += VWORD_SIZE;
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;

    if(ptr == resolve_symbol("evalparam")->start)
    {
        vword_t fn = context->read_word(p);
        p += VWORD_SIZE;
        vword_t l = context->read_word(p);
        p += VWORD_SIZE;

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

        //context->dump(p, 8);
    }
    else if(ptr == resolve_symbol("macro")->start)
    {
        p = ll_expand_macro(context, p);
        p = ll_expand(context, p);
    }
//	else if(ptr == resolve_symbol("function")->start)
//	{
//	}
    else
    {
        p = op;
    }

    return p;
}

