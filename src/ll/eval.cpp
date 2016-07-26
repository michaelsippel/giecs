#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <stackframe.h>

#include <lisp/parser.h>
#include <ll.h>

void ll_eval(StackFrame& stack)
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
        void (*f)(StackFrame&);
        fn.pop((vbyte_t*) &f, IWORD_SIZE);

        f(stack);
    }
}

void ll_deval(StackFrame& stack)
{
    vword_t num_list = stack.pop_word();
    vword_t addr_list = stack.pop_word();

    vword_t* list_index = (vword_t*) malloc(num_list * sizeof(vword_t));

	StackFrame liststack = StackFrame(stack.context, addr_list);

    int i;
    for(i = 0; i < num_list; i++)
    {
        list_index[i] = liststack.ptr();

        vword_t attr = liststack.pop_word();
        liststack.move( (attr == (vword_t)-1) ? VWORD_SIZE : attr );
    }

    for(i = num_list-1; i >= 0; i--)
    {
        vword_t list_addr = list_index[i];

		StackFrame s = StackFrame(stack.context, list_addr);

        vword_t attr = s.pop_word();
        if(attr == (vword_t)-1)
        {
            // execute
            vword_t w = s.pop_word();
			stack.push_word(w);

            ll_eval(stack);
        }
        else
        {
            stack.move(-attr);
            stack.context->copy(stack.ptr(), s.ptr(), attr);
			s.move(attr);
        }
    }

    free(list_index);

    ll_eval(stack);
}

