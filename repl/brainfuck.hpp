
#pragma once

#include <languages/brainfuck/brainfuck.hpp>

#include "language.hpp"

namespace repl
{
namespace lang
{

class Brainfuck : public Language
{
    public:
        Brainfuck()
        {
        }

        int parse(std::istream& stream)
        {
            /*   char str[512];
                 stream.get(str, 512);

                 mem.operate(addr_t(), this->limit, [](word_t& v)
                 {
                 v=word_t();
                 }, true);

                 brainfuck::SyntaxAccessor syntax(code_ptr);
                 syntax.parse(stream);
                 giecs::eval(code_ptr);
                 std::cout << std::endl;
            */
            return 1;
        }

        void name(char* buf)
        {
            strcpy(buf, "Brainfuck");
        }
}; // class Brainfuck

} // namespace lang

} // namespace repl

