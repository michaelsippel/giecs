#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <context.h>
#include <lisp/reader.h>
#include <ll.h>
#include <syscalls.h>

vword_t ll_exit(Context* context, vword_t p)
{
    vword_t val = context->read_word(p);

    printf("Exit with value %d\n", val);
    exit(val);

    return p + VWORD_SIZE;
}

vword_t ll_syscall(Context* context, vword_t p)
{
    vword_t regs_in[6];
    p += context->read(p, 6*VWORD_SIZE, (vbyte_t*) &regs_in);

    void* regs_out[6] = {(void*)regs_in[0],0,0,0,0,0};

    char* buf[5] = {NULL, NULL, NULL, NULL, NULL};
    size_t sizes[5] = {0, 0, 0, 0, 0};

    int i;
    for(i = 0; i < 5; i++)
    {
        vword_t ptr;
        vbyte_t c;

        int j;
        size_t size;

        switch(syscall_params[regs_in[0]][i])
        {
            case NONE:
            case INT:
                regs_out[i+1] = (void*)regs_in[i+1];
                break;

            case STRING:
                ptr = regs_in[i+1];

                size = 256;
                buf[i] = (char*) malloc(size);
                j = 0;
                do
                {
                    if(j > size)
                    {
                        size += 256;
                        buf[i] = (char*) realloc(buf[i], size);
                    }
                    context->read(ptr++, 1, &c);
                    buf[i][j] = c;
                    j++;
                }
                while(c != '\0');

                sizes[i] = (size_t)j;
                regs_out[i+1] = (void*) buf[i];

                break;

            default: // pointer
                break;
        }
    }

    uint32_t retv = 0;
    asm("int $0x80;"
        : "=a" (retv) : "a" (regs_out[0]),  "b" (regs_out[1]), "c"(regs_out[2]), "d"(regs_out[3]), "S"(regs_out[4]), "D"(regs_out[5]));

    p -= VWORD_SIZE;
    context->write_word(p, retv);

    for(i = 0; i < 5; i++)
    {
        context->write(regs_in[i+1], sizes[i], (vbyte_t*)buf[i]);
        free(buf[i]);
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

vword_t ll_prints(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;

    vbyte_t c = 0;
    do
    {
        context->read(ptr++, 1, (vbyte_t*) &c);
        write(1, &c, 1);
    }
    while(c != '\0');

    return p;
}

vword_t ll_resw(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);
    vword_t v = context->read_word(ptr);
    context->write_word(p, v);

    return p;
}

vword_t ll_setw(Context* context, vword_t p)
{
    vword_t ptr = context->read_word(p);
    p += VWORD_SIZE;
    vword_t v = context->read_word(p);
    p += VWORD_SIZE;

    context->write_word(ptr, v);

    return p;
}

vword_t ll_nop(Context* context, vword_t p)
{
    return p;
}

