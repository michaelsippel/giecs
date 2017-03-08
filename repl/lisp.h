
#pragma once

#include <istream>

#include <giecs/memory/context.h>
#include <lisp/ast.h>
#include <lisp/reader.h>
#include <lisp/parser.h>
#include <lisp/ast_write.h>
#include <lisp/context.h>

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
        Lisp(memory::Context<page_size, align_t> const& context_, addr_t limit_)
            : context(context_, limit_)
        {
        }

        ~Lisp()
        {
        }

        Language* parse(std::istream& stream)
        {
            auto ast_root = lisp::Reader<lisp::ast::List>::read(stream);
            if(! ast_root->empty())
            {
                this->context.reset();
                lisp::Parser<lisp::ast::List>::parse(*ast_root, this->context);
                this->context.eval();
            }

            return this;
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
        LispASM(memory::Context<page_size, align_t> const& context_, addr_t limit_)
            : context(context_, limit_)
        {
        }

        ~LispASM()
        {
        }

        Language* parse(std::istream& stream)
        {
            auto ast_root = lisp::Reader<lisp::ast::List>::read(stream);
            if(! ast_root->empty())
            {
                this->context.reset();
                lisp::asm_parse(*ast_root, this->context);
                this->context.eval();
            }

            return this;
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

