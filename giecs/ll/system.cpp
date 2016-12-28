#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/limits.h>
#include <libgen.h>

#include <giecs/context.h>
#include <lisp/reader.h>
#include <lisp/parser.h>
#include <giecs/ll.h>
#include <giecs/syscalls.h>

void ll_load(StackFrame& stack)
{
    vword_t ptr = stack.pop_word();

    int cwd = open(".", O_RDONLY);

    char path[PATH_MAX+1];
    stack.context->read_str(ptr, (vbyte_t*) &path);

    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        printf("can't open \"%s\"\n", path);
        return;
    }

    chdir(dirname(path));

    size_t length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* buf = (char*) malloc(length+1);
    read(fd, buf, length);
    buf[length] = '\0';

    close(fd);

    SNode* ast = new SNode(LIST, buf);
    if(! ast->subnodes->isEmpty())
    {
        stack.move(lisp_parse_size(ast));
        if(lisp_parse(stack.context, stack.ptr(), ast) > 0)
        {
            stack.move(-VWORD_SIZE);
            ll_eval(stack);
        }
    }

    fchdir(cwd);
    close(cwd);
}

void ll_syscall(StackFrame& stack)
{
    vword_t regs_in[6];
    stack.pop((vbyte_t*) &regs_in, 6*VWORD_SIZE);

    long sys_id = regs_in[0];
    void* regs_out[6] = {(void*)sys_id,0,0,0,0,0};

    vbyte_t* buf[5] = {NULL, NULL, NULL, NULL, NULL};
    size_t sizes[5] = {0, 0, 0, 0, 0};

    int i;
    for(i = 0; i < 5; i++)
    {
        vbyte_t c;
        int j;

        switch(syscall_params[sys_id][i])
        {
            case NONE:
            case INT:
                regs_out[i+1] =(void*)((uintptr_t)regs_in[i+1]);
                continue;

            case STRING:
                j = 0;
                do
                {
                    stack.context->read(regs_in[i+1]+j, 1, &c);
                    j++;
                }
                while(c != '\0');

                sizes[i] = (size_t)j;
                break;

            case PTR_L_BX:
                sizes[i] = regs_in[1];
                break;
            case PTR_L_CX:
                sizes[i] = regs_in[2];
                break;
            case PTR_L_DX:
                sizes[i] = regs_in[3];
                break;
            case PTR_L_SI:
                sizes[i] = regs_in[4];
                break;
            case PTR_L_DI:
                sizes[i] = regs_in[5];
                break;

            default: // pointer
                sizes[i] = syscall_params[sys_id][i] - PTR;
                break;
        }

        buf[i] = (vbyte_t*) malloc(sizes[i] * sizeof(vbyte_t));
        stack.context->read(regs_in[i+1], sizes[i], buf[i]);
        regs_out[i+1] = (void*) buf[i];
    }

    int retv = 0;
    asm("int $0x80;"
        : "=a" (retv) : "a" (regs_out[0]),  "b" (regs_out[1]), "c"(regs_out[2]), "d"(regs_out[3]), "S"(regs_out[4]), "D"(regs_out[5]));

    stack.push_word((vword_t) retv);

    for(i = 0; i < 5; i++)
    {
        if(buf[i] != NULL)
        {
            stack.context->write(regs_in[i+1], sizes[i], buf[i]);
            free(buf[i]);
        }
    }
}

void ll_addi(StackFrame& stack)
{
    vword_t v1 = stack.pop_word();
    vword_t v2 = stack.pop_word();

    vword_t ret = v1 + v2;
    stack.push_word(ret);
}

void ll_muli(StackFrame& stack)
{
    vword_t v1 = stack.pop_word();
    vword_t v2 = stack.pop_word();

    vword_t ret = v1 * v2;
    stack.push_word(ret);
}

void ll_subi(StackFrame& stack)
{
    vword_t v1 = stack.pop_word();
    vword_t v2 = stack.pop_word();

    vword_t ret = v1 - v2;
    stack.push_word(ret);
}

void ll_divi(StackFrame& stack)
{
    vword_t v1 = stack.pop_word();
    vword_t v2 = stack.pop_word();

    vword_t ret = v1 / v2;
    stack.push_word(ret);
}

void ll_printi(StackFrame& stack)
{
    vword_t v = stack.pop_word();
    printf("%d\n", v);
}

void ll_printb(StackFrame& stack)
{
    vbyte_t val = stack.pop_byte();
    printf("%s\n", val ? "true" : "false");
}

static vword_t rel_base = 0x0;

void ll_setrelative(StackFrame& stack)
{
    vword_t ob = rel_base;
    rel_base = stack.ptr() + VWORD_SIZE;

    ll_eval(stack);

    rel_base = ob;
}

void ll_pop(StackFrame& stack)
{
    vword_t poplen = stack.pop_word();

    printf("pop %d bytes\n", poplen);

    size_t len = stack.ptr() + VWORD_SIZE; // We assume the next call has no arguments on stack
    ll_eval(stack);
    len -= stack.ptr();

    printf("returned len %d\n", len);

    stack.context->copy(stack.ptr()+poplen, stack.ptr(), len);
    stack.move(-poplen);
}

void ll_resw(StackFrame& stack)
{
    vword_t ptr = stack.pop_word();
    if((int)ptr < 1)
        ptr = rel_base - ptr;

    vword_t v = stack.context->read_word(ptr);
    stack.push_word(v);
}

void ll_setw(StackFrame& stack)
{
    vword_t ptr = stack.pop_word();
    if((int)ptr < 1)
        ptr = rel_base - ptr;

    vword_t v = stack.pop_word();
    stack.context->write_word(ptr, v);
}

void ll_resb(StackFrame& stack)
{
    vword_t ptr = stack.pop_word();

    vbyte_t v;
    stack.context->read(ptr, 1, &v);

    stack.push_byte(v);
}

void ll_setb(StackFrame& stack)
{
    vword_t ptr = stack.pop_word();
    vbyte_t v = stack.pop_byte();

    stack.context->write(ptr, 1, &v);
}

void ll_nop(StackFrame& stack)
{
    return;
}

