
#pragma once

#include <giecs/memory/context.h>

#include <sstream>
#include <lisp/ast.h>
#include <lisp/reader.h>
#include <lisp/parser.h>

#include "language.h"

using namespace giecs;

namespace repl
{

namespace lang
{

template <int page_size, typename align_t, typename addr_t=align_t, typename word_t=addr_t>
class Lisp : public Language
{
    public:
        Lisp(memory::Context<page_size, align_t> const& context_)
            : context(context_),
              stack(this->context.template createStack<addr_t, word_t>())
        {
        }

        ~Lisp()
        {
        }

        Language* parse(char* str)
        {
            std::istream* stream = new std::istringstream(std::string(str));

            auto ast_root = lisp::Reader<lisp::ast::List>::read(*stream);
            std::cout << *ast_root << std::endl;

            lisp::Parser<lisp::ast::List>::parse(*ast_root);

//			this->stack.push(addr);
//			this->core.eval(this->stack);

            if(std::string(str) == "exit")
            {
                delete this;
                return NULL;
            }

            return this;
        }

        void name(char* buf)
        {
            strcpy(buf, "Lisp");
        }

    private:
        memory::Context<page_size, align_t> const& context;
        memory::accessors::Stack<page_size, align_t, addr_t, word_t> stack;
};

} // namespace lang

} // namespace repl

