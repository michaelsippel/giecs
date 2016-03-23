#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <ll.h>
#include <context.h>

#include <lisp/reader.h>
#include <lisp/parser.h>
#include <lisp/ll.h>

#include <brainfuck/parser.h>
#include <brainfuck/ll.h>

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
    Context* context = new Context(0x1000, 4096);

    // place stack at end of memory
    vword_t stack = 4096*0x100 - VWORD_SIZE;
    vword_t entry_point = 0x2000;

    init_lisp(context);
    init_brainfuck(context);

    // parse a simple program
    SNode* ast = new SNode(LIST, (char*) "12 deval 1 (-1 (12 brainfuck \"++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.\"))");
    asm_parse_list(context, entry_point, ast);

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
            size_t l = lisp_parse(context, entry_point, ast);
//            context->dump(entry_point, l/4);

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

