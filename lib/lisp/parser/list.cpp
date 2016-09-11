#include <stdio.h>

#include <context.h>
#include <lisp/parser.h>

/*
 * low-level lisp parse (no self evaluation, no macros)
 */
int asm_parse_list(Context* context, vword_t addr, SNode* ast)
{
    // needed space
    size_t len = ast->subnodes->numOfElements() * sizeof(vword_t);
    vword_t lptr = addr;

    ListIterator<SNode*> it = ListIterator<SNode*>(ast->subnodes);
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();

        switch(sn->type)
        {
            case SYMBOL:
            case INTEGER:
                // direcly insert word
                asm_parse(context, lptr, sn);
                break;

            case LIST:
            case STRING:
                // here we need pointers
                vword_t subaddr = addr + len;
                len += asm_parse(context, subaddr, sn);

                context->write_word(lptr, subaddr);
                break;
        }

        lptr += VWORD_SIZE;
        it.next();
    }

    return len;
}

/*
 * lisp parse with self-evaluation and macros
 */
int lisp_parse_list(Context* context, vword_t addr, SNode* ast)
{
    // needed space
    size_t len = VWORD_SIZE;

    ListIterator<SNode*> it = ListIterator<SNode*>(ast->subnodes);

    SNode* sn = it.getCurrent();
    it.next();

    int n;
    vword_t fn;
    size_t reqb;

    // first word
    switch(sn->type)
    {
        case INTEGER:
        case SYMBOL:
            n = lisp_parse(context, addr+VWORD_SIZE, sn);
            if(n < 0)
                return n;

            fn = context->read_word(addr+VWORD_SIZE);
            reqb = get_reqb(fn);
            if(reqb > 0)
            {
                vword_t v[3];
                v[0] = resolve_symbol("evalparam")->start;
                v[1] = fn;
                v[2] = reqb;
                context->write(addr+VWORD_SIZE, 3*VWORD_SIZE, (vbyte_t*) &v);

                len += 3*VWORD_SIZE;
            }
            else
            {
                len += VWORD_SIZE;
            }
            break;

        case LIST:
            n = lisp_parse(context, addr, sn);
            if(n < 0)
                return n;
            len = n;
            break;
    }

    // insert ast subnodes
    while(! it.isLast())
    {
        len += it.getCurrent()->write_vmem(context, addr+len);
        it.next();
    }

    // write command length
    context->write_word(addr, len - VWORD_SIZE);

    return len;
}

size_t lisp_parse_size(SNode* ast)
{
    size_t len = 0;
    ListIterator<SNode*>* it;
    struct symbol* s;

    switch(ast->type)
    {
        case INTEGER:
            len = VWORD_SIZE;
            break;

        case SYMBOL:
            s = resolve_symbol(ast->string);
            if(s == NULL)
                return 0;

            if(get_reqb(s->start) == 0)
                len = VWORD_SIZE;
            else
                len = 3*VWORD_SIZE;
            break;

        case STRING:
            len = VWORD_SIZE + ast->vmem_size();
            break;

        case LIST:
            len = VWORD_SIZE;
            it = new ListIterator<SNode*>(ast->subnodes);

            len += lisp_parse_size(it->getCurrent());
            if(it->getCurrent()->type == LIST)
                len -= VWORD_SIZE;
            it->next();

            while(! it->isLast())
            {
                len += it->getCurrent()->vmem_size();
                it->next();
            }
            delete it;
            break;
    }

    return len;
}

