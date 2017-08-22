
#include <iostream>
#include <memory>

#include <cstdlib>
#include <cstddef>
#include "linenoise/linenoise.h"

#include <giecs/bits.hpp>
#include <giecs/types.hpp>
#include <giecs/memory/context.hpp>

#include "language.hpp"
#include "forth.hpp"
//#include "lisp.hpp"
#include "brainfuck.hpp"

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
    auto core = Core<page_size, byte, iword>();

    std::shared_ptr<repl::Language> lang = nullptr;
    std::shared_ptr<repl::Language> store = nullptr;

    // Read-Eval-Print-Loop
    linenoiseHistoryLoad("history.txt");
    linenoiseHistorySetMaxLen(20);
    char* line;
    char name[64];

    bool running = true;
    while(running)
    {
        while(lang == nullptr)
        {
            std::cout << "(x) exit\n(a) Lisp\n(b) Lisp ASM\n(c) Forth\n(d) Brainfuck\n\n";

            char c;
            std::cin >> c;
            switch(c)
            {/*
                case 'a':
                    lang = std::make_shared<repl::lang::Lisp<page_size, byte, iword>>(context, core, 0x10000);
                    break;
                    
                case 'b':
                    lang = std::make_shared<repl::lang::LispASM<page_size, byte, iword>>(context, core, 0x10000);
                    break;
             */
                case 'c':
                    lang = std::make_shared<repl::lang::Forth<page_size, byte, iword>>(context, core, 0x8000);
                    break;

            case 'd':
                    lang = std::make_shared<repl::lang::Brainfuck<page_size, byte, iword>>(context, 0x8000);
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
            if(lang->parse(line) != 0)
            {
                store = lang;
                lang = nullptr;
            }

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

