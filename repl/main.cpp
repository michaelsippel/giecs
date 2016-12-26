#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/limits.h>
#include <libgen.h>

//#include <ll.h>
//#include <context.h>
//#include <stackframe.h>

//#include <lisp/reader.h>
//#include <lisp/parser.h>
//#include <lisp/ll.h>

//#include <brainfuck/parser.h>
//#include <brainfuck/ll.h>
#include <math.h>

#include <giecs/bits.h>
#include <giecs/types.h>
#include <giecs/memory/context.h>
#include <giecs/memory/accessors/linear.h>
#include <giecs/memory/accessors/stack.h>
#include <giecs/core.h>

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

using namespace giecs;

typedef Bits<8> byte;
typedef Bits<32> word;
typedef Int<32> iword;

typedef memory::accessors::Stack<1024, byte, iword, word> Stack;

void addi(Stack& stack)
{
    iword a = stack.pop<word>();
    iword b = stack.pop<word>();
    iword c = a + b;
    stack.push<word>(c);
}

void printi(Stack& stack)
{
    iword a = stack.pop<word>();
    printf("%d\n", (int)a);
}

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    // set up vm
    auto c1 = memory::Context<1024, byte>();
    auto core = Core<1024, byte, iword, word>();
    core.addOperation(1, printi);
    core.addOperation(2, addi);

    Stack stack = c1.createStack<iword, word>();

    stack << word(2);
    stack << word(5);

    stack[5] = 2;
    stack[6] = 10;
    stack[7] = 2;

    core.eval(stack);

    stack << word(1);
    core.eval(stack);

    return 0;
    /*
        // place stack at end of memory
        StackFrame stack = StackFrame(context, context->upper_limit());

        init_lisp(context);
        /*    init_brainfuck(context);


                // parse a simple program
                SNode* ast = new SNode(LIST, (char*) "12 deval 1 (-1 (12 brainfuck \"++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.\"))");
                asm_parse_list(context, 0x2000, ast);

                // set entry point and run
                context->write_word(stack-VWORD_SIZE, 0x2000);
                ll_eval(context, stack-VWORD_SIZE);
        */
    // loading scripts
    /*    int i;
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
                int cwd = open(".", O_RDONLY);
                chdir(dirname(argv[i]));

                stack.move(-lisp_parse_size(ast));
                lisp_parse(context, stack.ptr(), ast);
                stack.move(VWORD_SIZE);
                ll_eval(stack);

                fchdir(cwd);

                close(cwd);
                close(fd);
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
                vword_t s = stack.ptr();
                stack.move(lisp_parse_size(ast));
                if(lisp_parse(stack.context, stack.ptr(), ast) > 0)
                {
                    //                stack = ll_expand(context, stack);
                    stack.move(-VWORD_SIZE);
                    ll_eval(stack);

                    size_t l = s-stack.ptr();
                    if(l > 0)
                    {
                        printf("return: ");
                        switch(l)
                        {
                            case 1:
                                ll_printb(stack);
                                break;

                            case VWORD_SIZE:
                                ll_printi(stack);
                                break;

                            default:
                                context->dump(stack.ptr(), l/VWORD_SIZE);
                        }
                    }
                }
            }
            delete ast;
        }

        delete context;
    */
    return 0;
}

