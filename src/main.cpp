#include <stdio.h>
#include <string.h>
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
//    printf("Hello World!\n");

    // set up vm
    Context* context = new Context(0x1000, 64);

    add_symbol("eval", context->add_ll_fn(ll_eval));
    add_symbol("deval", context->add_ll_fn(ll_deval));
    add_symbol("nop", context->add_ll_fn(ll_nop));

//    add_symbol("if", context->add_ll_fn(ll_cond));
//    add_symbol("eq", context->add_ll_fn(ll_eq));

//    add_symbol("map", context->add_ll_fn(ll_map));
    add_symbol("exit", context->add_ll_fn(ll_exit));
    add_symbol("printi", context->add_ll_fn(ll_printi));
    add_symbol("printb", context->add_ll_fn(ll_printb));
    add_symbol("addi", context->add_ll_fn(ll_addi));

    // place stack at end of memory
    vword_t stack = 64*0x1000 - VWORD_SIZE;
    vword_t entry_point = 0x100;

    /*    init_brainfuck(context);
        parse_brainfuck(context, entry_point, (char*) "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.");
    */
    // parse a simple program
    SNode* ast = new SNode(LIST, (char*) "12 deval 2 (4 printi -1 (12 addi 2 3))");
    parse_list(context, entry_point, ast);

    // set entry point and run
    context->write_word(stack, entry_point);
    ll_eval(context, stack);

    // interpreter loop (read-eval-print)
    printf("\n\033[0;49;32mType S-expressions\033[0m\n");
    while(1)
    {
        write(1, "\033[0;49;32m>>> \033[0;49;33m", 24);
        char str[256];
        readline(0, str);
        write(1, "\033[0m", 4);

        SNode* ast = new SNode(LIST, str);
        if(! ast->subnodes->isEmpty())
        {
            //ast->dump();
            parse_list(context, entry_point, ast);

            context->write_word(stack, entry_point);
            vword_t s = ll_eval(context, stack);
            if(s <= stack)
                ll_printi(context, s);
        }
        delete ast;
    }

    delete context;

    return 0;
}

