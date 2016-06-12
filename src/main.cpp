#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

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
    vword_t stack = 4096*0x100;

    init_lisp(context);
    init_brainfuck(context);

    // parse a simple program
    SNode* ast = new SNode(LIST, (char*) "12 deval 1 (-1 (12 brainfuck \"++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.\"))");
    asm_parse_list(context, 0x2000, ast);

    // set entry point and run
    context->write_word(stack-VWORD_SIZE, 0x2000);
    ll_eval(context, stack-VWORD_SIZE);

    // loading scripts
    int i;
    for(i = 1; i < argc; i++)
    {
        int fd = open(argv[i], O_RDONLY);
        size_t length = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        char* buf = (char*) malloc(length+1);
        read(fd, buf, length);
        buf[length] = '\0';

        SNode* ast = new SNode(LIST, buf);
        if(! ast->subnodes->isEmpty())
        {
            stack -= lisp_parse_size(ast);
            lisp_parse(context, stack, ast);
            ll_eval(context, stack+VWORD_SIZE);
        }
    }

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
            vword_t s = stack;
            stack -= lisp_parse_size(ast);
            if(lisp_parse(context, stack, ast) > 0)
            {
                stack = ll_eval(context, stack+VWORD_SIZE);

                if(stack < s)
                {
                    printf("return: ");
                    switch(s-stack)
                    {
                        case 1:
                            ll_printb(context, stack);
                            break;

                        case VWORD_SIZE:
                            ll_printi(context, stack);
                            break;

                        default:
                            context->dump(stack, (s-stack)/VWORD_SIZE);
                    }
                }
            }
        }
        delete ast;
    }

    delete context;

    return 0;
}

