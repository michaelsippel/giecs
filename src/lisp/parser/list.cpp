#include <stdio.h>

#include <context.h>
#include <parser.h>

int parse_list(Context* context, vword_t addr, SNode* ast)
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
                parse(context, lptr, sn);
                break;

            case LIST:
            case STRING:
                // here we need pointers
                vword_t subaddr = addr + len;
                len += parse(context, subaddr, sn);

                context->write_word(lptr, subaddr);
                break;
        }

        lptr += sizeof(vword_t);
        it.next();
    }

    return len;
}

int parse_list_se(Context* context, vword_t addr, SNode* ast)
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
            parse(context, addr+VWORD_SIZE, sn);
            break;

        case LIST:
            context->write_word(addr+VWORD_SIZE, addr+len);
            len += parse_se(context, addr+len, sn);
            break;
    }

    return len;
}

