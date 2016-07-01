#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/limits.h>
#include <libgen.h>

#include <context.h>
#include <lisp/reader.h>
#include <lisp/parser.h>
#include <ll.h>
#include <syscalls.h>

vword_t ll_load(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;

    int cwd = open(".", O_RDONLY);

    char path[PATH_MAX+1];
    context->read_str(ptr, (vbyte_t*) &path);

    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        printf("can't open \"%s\"\n", path);
        return p;
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
        p -= lisp_parse_size(ast);
        if(lisp_parse(context, p, ast) > 0)
        {
            p = ll_eval(context, p+VWORD_SIZE);
        }
    }

    fchdir(cwd);
    close(cwd);

    return p;
}

vword_t ll_syscall(Context* context, vword_t p)
{
    vword_t regs_in[6];
    p += context->read(p, 6*VWORD_SIZE, (vbyte_t*) &regs_in);

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
                    context->read(regs_in[i+1]+j, 1, &c);
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
        context->read(regs_in[i+1], sizes[i], buf[i]);
        regs_out[i+1] = (void*) buf[i];
    }

    uint32_t retv = 0;
    asm("int $0x80;"
        : "=a" (retv) : "a" (regs_out[0]),  "b" (regs_out[1]), "c"(regs_out[2]), "d"(regs_out[3]), "S"(regs_out[4]), "D"(regs_out[5]));

    p -= VWORD_SIZE;
    context->write_word(p, retv);

    for(i = 0; i < 5; i++)
    {
        if(buf[i] != NULL)
        {
            context->write(regs_in[i+1], sizes[i], buf[i]);
            free(buf[i]);
        }
    }

    return p;
}

vword_t ll_addi(Context* context, vword_t p)
{
    vword_t val[2];
    context->read(p, 2*VWORD_SIZE, (vbyte_t*) &val);

    vword_t ret = val[0] + val[1];
    context->write_word(p+VWORD_SIZE, ret);

    return p + VWORD_SIZE;
}

vword_t ll_muli(Context* context, vword_t p)
{
    vword_t val[2];
    context->read(p, 2*VWORD_SIZE, (vbyte_t*) &val);

    vword_t ret = val[0] * val[1];
    context->write_word(p+VWORD_SIZE, ret);

    return p + VWORD_SIZE;
}

vword_t ll_subi(Context* context, vword_t p)
{
    vword_t val[2];
    context->read(p, 2*VWORD_SIZE, (vbyte_t*) &val);

    vword_t ret = val[0] - val[1];
    context->write_word(p+VWORD_SIZE, ret);

    return p + VWORD_SIZE;
}

vword_t ll_divi(Context* context, vword_t p)
{
    vword_t val[2];
    context->read(p, 2*VWORD_SIZE, (vbyte_t*) &val);

    vword_t ret = val[0] / val[1];
    context->write_word(p+VWORD_SIZE, ret);

    return p + VWORD_SIZE;
}

vword_t ll_printi(Context* context, vword_t p)
{
    vword_t val = context->read_word(p);
    printf("%d\n", val);

    return p + VWORD_SIZE;
}

vword_t ll_printb(Context* context, vword_t p)
{
    vbyte_t val;
    context->read(p, 1, &val);
    printf("%s\n", val ? "true" : "false");
    return p + 1;
}

static vword_t rel_base = 0x0;

vword_t ll_setrelative(Context* context, vword_t p)
{
    vword_t poplen = context->read_word(p);
    p += VWORD_SIZE;

    vword_t ob = rel_base;
    rel_base = p+VWORD_SIZE;

    p = ll_eval(context, p);

    size_t len = rel_base - p;
    context->copy(p+poplen, p, len);
    p += poplen;

    rel_base = ob;
    return p;
}

vword_t ll_resw(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);

    //printf("resw %d, relbase %d\n", ptr, rel_base);
    if((int)ptr < 1)
        ptr = rel_base - ptr;

    vword_t v = context->read_word(ptr);
    context->write_word(p, v);

    return p;
}

vword_t ll_setw(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;

    if((int)ptr < 1)
        ptr = rel_base - ptr;

    vword_t v = context->read_word(p);
    p += VWORD_SIZE;
    context->write_word(ptr, v);

    return p;
}

vword_t ll_resb(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;

    vbyte_t v;
    context->read(ptr, 1, &v);

    p -= 1;
    context->write(p, 1, &v);

    return p;
}

vword_t ll_setb(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;
    vbyte_t v;
    context->read(p, 1, &v);
    p += 1;

    context->write(ptr, 1, &v);

    return p;
}


vword_t ll_nop(Context* context, vword_t p)
{
    return p;
}

