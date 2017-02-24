
#include <iostream>

#include <cstdlib>
#include <cstddef>
#include "linenoise/linenoise.h"

#include <giecs/bits.h>
#include <giecs/types.h>
#include <giecs/memory/context.h>

#include "language.h"
#include "forth.h"
#include "lisp.h"
#include "brainfuck.h"

using namespace giecs;

int main(int argc, char** argv)
{
    // set up vm
    size_t const page_size = 4096;
    size_t const word_width = 32;

    typedef Bits<8> byte;
    typedef Bits<word_width> word;
    typedef Int<word_width> iword;
    auto const context = memory::Context<page_size, byte>();

    repl::Language* lang = NULL;

    // Read-Eval-Print-Loop
    linenoiseHistoryLoad("history.txt");
    linenoiseHistorySetMaxLen(20);
    char* line;
    char name[64];

    bool running = true;
    while(running)
    {
        while(lang == NULL)
        {
            std::cout << "(x) exit\n(a) Lisp\n(b) Forth\n(c) Brainfuck\n\n";

            char c;
            std::cin >> c;
            switch(c)
            {
                case 'a':
                    lang = new repl::lang::Lisp<page_size, byte, iword>(context);
                    break;

                case 'b':
                    lang = new repl::lang::Forth<page_size, byte, iword>(context, 0x8000);
                    break;

                case 'c':
                    lang = new repl::lang::Brainfuck<page_size, byte, iword>(context, 0x8000);
                    break;

                case 'x':
                    return 0;
            }

            if(lang != NULL)
            {
                lang->name(name);
                strcat(name, " >>> ");
            }
        }

        if((line = linenoise(name)) != NULL)
        {
            lang = lang->parse(line);

            if(line[0] != '\0')
            {
                linenoiseHistoryAdd(line);
                linenoiseHistorySave("history.txt");
            }
            linenoiseFree(line);
        }
    }

    return 0;
}

