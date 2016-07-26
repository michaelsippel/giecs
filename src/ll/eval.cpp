#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <stackframe.h>

#include <lisp/parser.h>
#include <ll.h>

vword_t ll_eval(Context* context, vword_t p)
{
    return p;
}

void ll_eval(StackFrame stack)
{
    vword_t addr = stack.pop_word();

    StackFrame fn = StackFrame(stack.context, addr);
    vword_t len = fn.pop_word();

    if(len != (vword_t)-1)
    {
        // apply parameters
        stack.move(-len);
        stack.context->copy(stack.ptr(), fn.ptr(), len);

        // eval again
        ll_eval(stack);
    }
    else
    {
        // call low-level function
        vword_t (*f)(Context*, vword_t);
        fn.pop((vbyte_t*) &f, IWORD_SIZE);

        f(stack.context, stack.ptr());
    }
}

vword_t ll_deval(Context* context, vword_t p)
{
    vword_t in[2];
    context->read(p, 2*VWORD_SIZE, (vbyte_t*) &in);
    vword_t num_list = in[0];
    vword_t addr_list = in[1];

    p += 2 * VWORD_SIZE;

    vword_t* list_index = (vword_t*) malloc(num_list * sizeof(vword_t));

    int i;
    for(i = 0; i < num_list; i++)
    {
        vword_t attr = context->read_word(addr_list);

        list_index[i] = addr_list;

        addr_list += VWORD_SIZE;
        addr_list += (attr == (vword_t)-1) ? VWORD_SIZE : attr;
    }

    for(i = num_list-1; i >= 0; i--)
    {
        vword_t list_addr = list_index[i];
        vword_t attr = context->read_word(list_addr);
        list_addr += VWORD_SIZE;

        if(attr == (vword_t)-1)
        {
            // execute
            p -= VWORD_SIZE;
            vword_t w = context->read_word(list_addr);
            context->write_word(p, w);

            p = ll_eval(context, p);
        }
        else
        {
            p -= attr;
            context->copy(p, list_addr, attr);
        }
    }

    free(list_index);

    p = ll_eval(context, p);
    return p;
}

