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

