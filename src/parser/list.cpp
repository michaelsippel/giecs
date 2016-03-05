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

    size_t len = 4*VWORD_SIZE;
    size_t j = 0;
    vword_t lptr = addr + len;

    len += 2 * n * VWORD_SIZE;

    ListIterator<SNode*> it = ListIterator<SNode*>(ast->subnodes);
    while(! it.isLast())
    {
        SNode* sn = it.getCurrent();
        vword_t subaddr;
        vword_t m;

        switch(sn->type)
        {
            case SYMBOL:
            case INTEGER:
                // direcly insert word
                context->write_word(lptr + (j++)*VWORD_SIZE, VWORD_SIZE);
                parse_se(context, lptr + (j++)*VWORD_SIZE, sn);
                break;

            case LIST:
            case STRING:
                subaddr = addr+len;
                len += parse_se(context, subaddr, sn);

                m = (j == 0 || sn->type == STRING) ? VWORD_SIZE : -1;

                context->write_word(lptr + (j++)*VWORD_SIZE, m);
                context->write_word(lptr + (j++)*VWORD_SIZE, subaddr);
                break;
        }

        it.next();
    }

    vword_t v[4];
    v[0] = 3 * VWORD_SIZE;
    v[1] = resolve_symbol("deval");
    v[2] = n;
    v[3] = lptr;

    context->write(addr, 4*VWORD_SIZE, (vbyte_t*) &v);

    return len;
}

