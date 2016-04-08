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

        lptr += sizeof(vword_t);
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
//    size_t n = ast->subnodes->numOfElements();
    size_t len;

    ListIterator<SNode*> it = ListIterator<SNode*>(ast->subnodes);

    SNode* sn = it.getCurrent();
    it.next();

    // first word
    switch(sn->type)
    {
        case INTEGER:
        case SYMBOL:
            len = VWORD_SIZE + lisp_parse(context, addr+VWORD_SIZE, sn);
            break;

        case LIST:
            len = lisp_parse(context, addr, sn);
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

    switch(ast->type)
    {
        case INTEGER:
            len = VWORD_SIZE;
            break;

        case SYMBOL:
            if(resolve_symbol(ast->string).reqb == 0)
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

