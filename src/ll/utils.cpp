#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <ll.h>

void ll_cond(StackFrame& stack)
{
    vbyte_t c = stack.pop_byte();
    vword_t v0 = stack.pop_word();

    if(c)
    {
        stack.move(-VWORD_SIZE);
        stack.push_word(v0);
    }
}

void ll_eqw(StackFrame& stack)
{
    vword_t v0 = stack.pop_word();
    vword_t v1 = stack.pop_word();

    vbyte_t r = (v0 == v1) ? 1 : 0;
    stack.push_byte(r);
}

void ll_eqb(StackFrame& stack)
{
    vbyte_t v0 = stack.pop_byte();
    vbyte_t v1 = stack.pop_byte();

    vbyte_t r = (v0 == v1) ? 1 : 0;
    stack.push_byte(r);
}

void ll_map(StackFrame& stack)
{
    vword_t fn_addr = stack.pop_word();
    vword_t list_len = stack.pop_word();
    vword_t entry_size = stack.pop_word();
    vword_t list_addr = stack.pop_word();

    size_t l = list_len * entry_size;

    vbyte_t* list = (vbyte_t*) malloc(l * sizeof(vbyte_t));
    stack.context->read(list_addr, l, list);

    size_t sl = entry_size + VWORD_SIZE;
    vbyte_t* dest = (vbyte_t*) malloc(sl * sizeof(vbyte_t));
    *((vword_t*) dest) = fn_addr;

    int i,j;
    for(i = 0; i < list_len; i++)
    {
        vbyte_t* d = dest + VWORD_SIZE;
        for(j = 0; j < entry_size; j++)
            *d++ = list[i*entry_size + j];

        stack.push(dest, sl);

        ll_eval(stack);
    }

    free(dest);
    free(list);
}

