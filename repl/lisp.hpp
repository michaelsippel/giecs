
#pragma once

#include <istream>

#include <giecs/memory/context.hpp>
#include <lisp/ast.hpp>
#include <lisp/reader.hpp>
#include <lisp/parser.hpp>
#include <lisp/ast_write.hpp>
#include <lisp/context.hpp>

#include "language.hpp"

using namespace giecs;

namespace repl
{

namespace lang
{

template <int page_size, typename align_t, typename addr_t=align_t, typename word_t=addr_t>
class Lisp : public Language
{
    public:
        Lisp(memory::Context<page_size, align_t> const& context_, Core<page_size, align_t, addr_t>& core_, addr_t limit_)
            : context(context_, core_, limit_)
        {
        }

        ~Lisp()
        {
        }

        int parse(std::istream& stream)
        {
            auto ast_root = lisp::Reader<lisp::ast::List>::read(stream);
            if(! ast_root->empty())
            {
                this->context.reset();
                lisp::Parser<lisp::ast::List>::parse(*ast_root, this->context);
                this->context.eval();
            }

            return 0;
        }

        void name(char* buf)
        {
            strcpy(buf, "Lisp");
        }

    private:
        lisp::Context<page_size, align_t, addr_t, word_t> context;
}; // class Lisp

template <int page_size, typename align_t, typename addr_t=align_t, typename word_t=addr_t>
class LispASM : public Language
{
    public:
        LispASM(memory::Context<page_size, align_t> const& context_, Core<page_size, align_t, addr_t>& core_, addr_t limit_)
            : context(context_, core_, limit_)
        {
        }

        ~LispASM()
        {
        }

        int parse(std::istream& stream)
        {
            auto ast_root = lisp::Reader<lisp::ast::List>::read(stream);
            if(! ast_root->empty())
            {
                this->context.reset();
                lisp::asm_parse(*ast_root, this->context);
                this->context.eval();
            }

            return 0;
        }

        void name(char* buf)
        {
            strcpy(buf, "Lisp ASM");
        }

    private:
        lisp::Context<page_size, align_t, addr_t, word_t> context;
}; // class LispASM

} // namespace lang

} // namespace repl

