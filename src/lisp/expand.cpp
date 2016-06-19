
#include <context.h>
#include <lisp/parser.h>

// compile to lower level to avoid reparsing
vword_t ll_expand(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p+VWORD_SIZE);

    if(ptr == resolve_symbol("evalparam")->start)
    {
        p += 2*VWORD_SIZE;

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
            n += VWORD_SIZE;
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
            sn->dump();

            vword_t resbuf[3];
            resbuf[0] = 2*VWORD_SIZE;
            resbuf[1] = resolve_symbol("resw")->start;

            switch(sn->type)
            {
                case INTEGER:
                    *lp++ = VWORD_SIZE;
                    *lp++ = sn->integer;
                    break;

                case SYMBOL:
                    p -= VWORD_SIZE;
                    asm_parse(context, p, sn);

                    p -= 2*VWORD_SIZE;
                    context->write(p, 2*VWORD_SIZE, (vbyte_t*) &resbuf);

                    *lp++ = -1;
                    *lp++ = p;
                    break;

                case LIST:
                    p -= lisp_parse_size(sn);
                    lisp_parse(context, p, sn);
                    *lp++ = -1;
                    *lp++ = p;
                    break;
            }

            it.next();
        }

        p -= 2*VWORD_SIZE*j;
        context->write(p, 2*VWORD_SIZE*j, listbuf);

        free(listbuf);
        delete plist;

        vword_t b0[4];
        b0[0] = 3*VWORD_SIZE;
        b0[1] = resolve_symbol("deval")->start;
        b0[2] = j;
        b0[3] = p;

        p -= 4*VWORD_SIZE;
        context->write(p, 4*VWORD_SIZE, (vbyte_t*) &b0);

        //	context->dump(p, 8);
    }

    return p;
}

