#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/limits.h>
#include <libgen.h>

#include <ll.h>
#include <context.h>
#include <stackframe.h>

#include <lisp/reader.h>
#include <lisp/parser.h>
#include <lisp/ll.h>

#include <brainfuck/parser.h>
#include <brainfuck/ll.h>
#include <math.h>


#include <bits.h>
#include <memory/context.h>
#include <memory/block.h>
#include <memory/accessor.h>

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

int main(int argc, char** argv)
{
//    printf("Hello World!\n");

    // set up vm
    memory::Context* c1 = new memory::Context();

    typedef Bits<6> byte;
    typedef Bits<24> word;

    struct Index
    {
        int x,y;
        Index()
            :x(0),y(0)
        {}

        Index(int x_, int y_)
            : x(x_), y(y_)
        {
            this->y += this->x / 4;
            this->x %= 4;
        }

        Index operator+(Index const& i) const
        {
            return Index(this->x+i.x, this->y+i.y);
        }

        bool operator<(Index const& i) const
        {
            return ((int)*this  < (int)i);
        }

        Index& operator++()
        {
            *this = Index(this->x+1, this->y);
            return *this;
        }

        operator int() const
        {
            return (this->x + this->y*4);
        }
    };

    struct Array
    {
        byte* ptr;

        Array(void const* ptr_)
            :ptr((byte*)ptr_)
        {}

        byte& operator[] (Index const& i) const
        {
            return this->ptr[(int)i];
        }

        byte& operator* () const
        {
            return *this->ptr;
        }

        Array& operator++()
        {
            this->ptr++;
            return *this;
        }
    };

    auto acc = memory::accessors::Linear<Index, byte, Array, Index>(c1);

    Array buf = Array(malloc(0x1000));
    Index len = Index(0,4);

    int a = 0;
    for(Index i = Index(); i < len; ++i)
    {
        ++a;
        buf[i] = a;
    }

    Index l = acc.write(Index(), len, buf);

    for(Index i = Index(); i < len; ++i)
    {
        byte b = acc[i];
        printf("%d\n", (int)b);
    }

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

