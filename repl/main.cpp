
#include <cstdlib>
#include <cstddef>
#include "linenoise/linenoise.h"

#include <giecs/bits.h>
#include <giecs/types.h>
#include <giecs/memory/context.h>
#include <giecs/memory/accessors/linear.h>
#include <giecs/memory/accessors/stack.h>
#include <giecs/ll/arithmetic.h>
#include <giecs/ll/io.h>
#include <giecs/ll/system.h>
#include <giecs/core.h>

using namespace giecs;

int main(int argc, char** argv)
{
    // set up vm
    size_t const page_size = 1024;
    size_t const word_width = 32;

    typedef Bits<8> byte;
    typedef Bits<word_width> word;
    typedef Int<word_width> iword;
    auto context = memory::Context<page_size, byte>();

    linenoiseHistoryLoad("history.txt");
    linenoiseHistorySetMaxLen(20);

    // Read-Eval-Print-Loop
    char* line;
    while((line = linenoise("repl> ")) != NULL)
    {
        printf("%s\n", line);

        if(line[0] != '\0')
        {
            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt");
        }
        linenoiseFree(line);
    }

    return 0;
}

