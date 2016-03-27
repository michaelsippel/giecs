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
    size_t n = ast->subnodes->numOfElements();
    size_t len = (n+1) * VWORD_SIZE;

    ListIterator<SNode*> it = ListIterator<SNode*>(ast->subnodes);

    SNode* sn = it.getCurrent();
    it.next();

    vword_t subaddr = addr + 2*VWORD_SIZE;

    // pointers to ast subnodes
    while(! it.isLast())
    {
        context->write_word(subaddr, addr+len);
        len += it.getCurrent()->write_vmem(context, addr+len);

        subaddr += VWORD_SIZE;
        it.next();
    }

    // write command length
    context->write_word(addr, n*VWORD_SIZE);

    // first word
    switch(sn->type)
    {
        case INTEGER:
        case SYMBOL:
            lisp_parse(context, addr+VWORD_SIZE, sn);
            break;

        case LIST:
            context->write_word(addr+VWORD_SIZE, addr+len);
            len += lisp_parse(context, addr+len, sn);
            break;
    }

    return len;
}

size_t lisp_parse_size(SNode* ast)
{
    size_t len = 0;
    ListIterator<SNode*>* it;

    switch(ast->type)
    {
        case INTEGER:
        case SYMBOL:
            len = VWORD_SIZE;
            break;

        case STRING:
            len = VWORD_SIZE + ast->vmem_size();
            break;

        case LIST:
            len = (ast->subnodes->numOfElements()+1) * VWORD_SIZE;
            it = new ListIterator<SNode*>(ast->subnodes);

            if(it->getCurrent()->type == LIST)
                len += lisp_parse_size(it->getCurrent());
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

