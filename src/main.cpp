#include <stdio.h>
#include <unistd.h>

#include <ll.h>
#include <context.h>
#include <reader.h>
#include <parser.h>

void readline(int fd, char* str)
{
    char c=0;
    while(1)
    {
        read(fd, &c, 1);

        if(c != '\n')
            *str++ = c;
        else
        {
            *str = '\0';
            break;
        }
    }
}

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    // set up vm
    Context* context = new Context(0x1000);

    add_symbol("eval", context->add_ll_fn(ll_eval));
    add_symbol("deval", context->add_ll_fn(ll_deval));

    add_symbol("if", context->add_ll_fn(ll_cond));
    add_symbol("eq", context->add_ll_fn(ll_eq));
    add_symbol("map", context->add_ll_fn(ll_map));

    add_symbol("exit", context->add_ll_fn(ll_exit));
    add_symbol("printi", context->add_ll_fn(ll_printi));
    add_symbol("printb", context->add_ll_fn(ll_printb));
    add_symbol("addi", context->add_ll_fn(ll_addi));

    // parse a simple program
    SNode* ast = new SNode(LIST, (char*) "16 deval printi 1 (-1 (12 addi 2 3))");
    parse_list(context, 0x100, ast);
    // set entry point
    *((vword_t*) context->base(0x1000-sizeof(vword_t))) = 0x100;
    // run
    ll_eval(context, 0x1000-sizeof(vword_t));

    // interpreter loop (read-eval-print)
    printf("\n\033[0;49;32mType S-expressions\033[0m\n");
    while(1)
    {
        write(1, "\033[0;49;32m>>> \033[0;49;33m", 24);
        char str[256];
        readline(0, str);
        write(1, "\033[0m", 4);

        SNode* ast = new SNode(LIST, str);
//        ast->dump();
        parse_list(context, 0x100, ast);

        *((vword_t*) context->base(0x1000-sizeof(vword_t))) = 0x100;
        vword_t top = ll_eval(context, 0x1000-sizeof(vword_t));
        if(top <= (0x1000-sizeof(vword_t)))
        {
            printf("direct output: %d\n", *((vword_t*)context->base(top)));
        }

        delete ast;
    }

    delete context;

    return 0;
}

