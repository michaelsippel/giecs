
#pragma once

#include <giecs/memory/context.h>

#include <lisp/ast.h>

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
            auto a = std::make_shared<lisp::ast::Atom<int> >(123);
            auto b = std::make_shared<lisp::ast::Atom<float> >(3.14159);
            auto c = std::make_shared<lisp::ast::Atom<std::string> >("Hello");

            auto ast_root = std::make_shared<lisp::ast::List>();

            ast_root->addNode(a);
            ast_root->addNode(b);
            ast_root->addNode(c);

            std::cout << *ast_root << std::endl;

//			this->stack.push(addr);
//			this->core.eval(this->stack);

            std::cout << str << "\n";

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

