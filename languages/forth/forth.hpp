
#pragma once

#include <istream>
#include <string>
#include <map>
#include <boost/algorithm/string.hpp>
#include <giecs/vm.hpp>
#include <giecs/ll/arithmetic.hpp>
#include <giecs/ll/cond.hpp>
#include <giecs/ll/io.hpp>

#include "language.hpp"

namespace forth
{

enum WordType
{
    Lit,
    SubWord,
};

class Context
{
    public:
        giecs::memory::Context<page_size, align_t> const& context;

        struct Entry
        {
            addr_t link;
            bool immediate;
        };
        std::map<std::string, Entry> dictionary;

        enum Primitive
        {
            noop,
            // flow
            branch,
            exit,
            // meta
            compile,
            execute,
            create,
            // stack
            push,
            drop,
            dup,
            // memory
            load,
            store,
            // logic
            andl,
            orl,
            xorl,
            // integer
            addi,
            subi,
            muli,
            divi,
        };

        Context()
        {
            ForthWord define = ForthWord();
            ForthWord immediate = ForthWord();
            this->add(":", define);
            this->add("immediate", immediate);
            ": [ immediate 0 state ! exit"
            ": ] immediate 1 state ! exit"

            ": ; immediate ' exit , ] exit"
            ": here h @ ;"
            ": >mark here 0 , ;"
            ": if immediate ' ?branch , >mark ;"
            ": else immediate ' branch , >mark swap dup here swap - swap ! ;"
            ": then immediate dup here swap - swap ! ;"
            ": ? @ . ;"
            ": cr 10 emit ;"
        }
};

class ForthSyntaxAccessor : public giecs::memory::accessors::Stream<page_size, align_t>
{
    public:
        ForthSyntax() {}

        // parse buffer to internal structure
        void write_page ()
        {
            ForthWord words;
            std::istream& stream = getBlock();

            while(! stream.eof())
            {
                std::string word_str;
                word_str << stream;

                bool compile = state;

                // find
                auto it = this->dictionary.find(word_str);
                if(it != this->dictionary.end())
                {
                    addr_t addr = it->second.link;
                    compile |= it->second.compile;
                    words.push(addr);
                }
                else
                {
                    // try if its a number
                    try
                    {
                        word_t a = std::stroi(word_str);

                        words.push(lit);
                        words.push(a);
                    }
                    catch(std::invalid_argument const&)
                    {
                        std::cout << "Error: undefined symbol \"" << word << "\"!\n";
                    }
                }

                if(!compile)
                {
                    // execute
                    // Create Code Accessor from Word
                    // Core->Execute
                    // create new word from result on the stack
                }
            }
            page = words->getPage();
        }
};

template <std::size_t page_size, typename align_t, typename word_t>
class ForthWord : public giecs::memory::accessors::Queue<page_size, align_t, std::pair<WordType, word_t>>
{
    public:
        ForthWord(giecs::memory::Context<page_size, align_t> const& context_)
            : giecs::memory::accessors::Queue<page_size, align_t, std::pair<WordType, word_t>>(context_)
        {}

        ForthWord(ForthWord const& parent, word_t addr)
            : giecs::memory::accessors::Queue<page_size, align_t, std::pair<WordType, word_t>>(giecs::memory::accessors::Deque<page_size, align_t, std::size_t, std::pair<WordType, word_t>>(parent.c, std::ptrdiff_t(addr)))
        {}

        // Debug
        void print(void)
        {
            std::cout << "Begin Forth-Word" << std::endl;
            while(! this->empty())
            {
                auto p = this->front();
                switch(p.first)
                {
                    case Lit:
                        std::cout << "Lit: " << int(p.second) << std::endl;
                        break;

                    case SubWord:
                    {
                        ForthWord sw = giecs::memory::make_accessor<ForthWord>(*this, p.second);
                        sw.print();
                    }
                    break;
                }
                this->pop();
            }
            std::cout << "End Forth-Word" << std::endl;
        }
};

} // namespace forth

template <std::size_t page_size, typename align_t, typename word_t>
class Forth : public Language
{
    public:
        Forth(giecs::memory::Context<page_size, align_t> const& context_, giecs::Core<page_size, align_t>& core_)
            : context(context_), core(core_)
        {
            auto w1 = ForthWord<page_size, align_t, word_t>(this->context);
            w1.push(std::make_pair(WordType::Lit, word_t(2)));
            w1.push(std::make_pair(WordType::Lit, word_t(3)));
            w1.push(std::make_pair(WordType::SubWord, word_t(100)));

            auto w2 = ForthWord<>(this->context);

            w1.print();
        }

        ~Forth()
        {
        }

        using Language::parse;
        int parse(std::istream& stream)
        {
            auto reader = giecs::memory::make_accessor<Reader>(context, reference, stream, symbols);
            auto code = giecs::memory::make_accessor<Compiler>(context, reference);
            this->core.execute(code);

            std::cout << " ok" << std::endl;
            return 1;
        }

        void name(char* buf)
        {
            strcpy(buf, "Forth");
        }

    private:

};

} // namespace lang

} // namespace repl

