
#pragma once

#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include <string>

#include <languages/forth/vm.hpp>

namespace forth
{

template <typename Cell>
class Bootstrap : public VM<Cell, std::array<Cell, 10000>, std::vector<Cell>, std::vector<Cell>>
{
    public:
        using fVM = VM<Cell, std::array<Cell, 10000>, std::vector<Cell>, std::vector<Cell>>;
        using Instruction = typename fVM::Instruction;
        using Opcode = typename Instruction::Opcode;

        std::istream* stream;
        struct Entry
        {
            bool immediate;
            Cell xt;
        };

        std::map<std::string, Entry> dictionary;

        void compile(Cell a)
        {
            this->mem[this->here()++] = a;
        }
        void compile(std::string name)
        {
            this->compile(this->dictionary[name].xt);
        }
        void compile_literal(Cell a)
        {
            this->compile("push");
            this->compile(a);
        }
        void compile_prim(std::string name, Opcode op, bool immediate=false)
        {
            Cell addr = here()++;
            this->dictionary[name] = {immediate, addr};
            this->mem[addr] = op;
        }
        void compile_comp(std::string name, std::vector<std::string> prg, bool immediate=false)
        {
            this->compile_prim(name, Opcode::compose, immediate);
            for(std::string const& word : prg)
                this->read_word(word, true);
        }

        // Simple extern program
        template <typename Functor>
        struct Ext : public giecs::ProgramBase
        {
                struct Evaluator : public giecs::EvaluatorBase
                {
                    Functor fn;
                    Bootstrap& bs;
                    Evaluator(Functor const& fn_, Bootstrap& bs_)
                        : fn(fn_), bs(bs_)
                    {}
                    void eval(ProgramBase& prg)
                    {
                        this->fn(this->bs);
                    }
                };

                Evaluator e;

                Ext(Functor const& f, Bootstrap& bs)
                    : e(f, bs)
                {
                }

                giecs::EvaluatorBase* createEvaluator(void)
                {
                    return &this->e;
                }

                giecs::ProgramBase* next(void)
                {
                    // not returning works for immediate mode
                    return nullptr;
                }
        };

        template <typename Functor>
        void compile_ext(std::string name, Functor const& fn, bool immediate=false)
        {
            giecs::ProgramBase* prg = new Ext<Functor>(fn, *this);
            Cell addr = here()++;
            this->programs[addr] = prg;
            this->compile_prim(name, Opcode::compose, immediate);
            this->compile_literal(addr);
            this->compile("execute");
        }

    public:
        Bootstrap()
        {
            this->compiling() = 0;
            this->here() = 0x100;

            compile_prim("push", Opcode::push);
            compile_prim("?branch", Opcode::branch);
            compile_prim("exit", Opcode::exit);
            compile_prim("!", Opcode::store);
            compile_prim("@", Opcode::load);
            compile_prim("dup", Opcode::dup);
            compile_prim("over", Opcode::over);
            compile_prim("drop", Opcode::drop);
            compile_prim("swap", Opcode::swap);
            compile_prim(">r", Opcode::pushr);
            compile_prim("r>", Opcode::popr);

            compile_prim("negate", Opcode::noti);
            compile_prim("and", Opcode::andi);
            compile_prim("or", Opcode::ori);
            compile_prim("xor", Opcode::xori);
            compile_prim("+", Opcode::addi);
            compile_prim("-", Opcode::subi);
            compile_prim("*", Opcode::muli);
            compile_prim("/", Opcode::divi);
            compile_prim("<", Opcode::lti);
            compile_prim(">", Opcode::gti);
            compile_prim("=", Opcode::eq);

            compile_prim("emit", Opcode::emit);
            compile_prim(".", Opcode::printi);

            compile_comp("execute", {">r", "exit"});

            compile_ext(":", [](Bootstrap& bs)
            {
                std::string name;
                (*bs.stream) >> name;
                std::cout << "DEFINE " << name << std::endl;
                bs.compile_prim(name, Opcode::compose);
                bs.compiling() = 1;
            }, true);
            compile_ext("create", [](Bootstrap& bs)
            {
                std::string name;
                (*bs.stream) >> name;
                std::cout << "CREATE " << name << std::endl;
                // TODO
                bs.compile_prim(name, Opcode::compose);
                bs.compile_literal(bs.here()+3);
                bs.compile("exit");
            });
            compile_ext("immediate", [](Bootstrap& bs)
            {
                for(auto& e : bs.dictionary)
                {
                    Cell addr = bs.here() - 1;
                    if(e.second.xt == addr)
                    {
                        e.second.immediate = true;
                        break;
                    }
                }
            }, true);
            compile_ext("[']", [](Bootstrap& bs)
            {
                std::string name;
                (*bs.stream) >> name;
                bs.compile_literal(bs.dictionary[name].xt);
            }, true);
            compile_ext("'", [](Bootstrap& bs)
            {
                std::string name;
                (*bs.stream) >> name;
                bs.compile_literal(bs.dictionary[name].xt);
            }, true);
            compile_ext(";", [](Bootstrap& bs)
            {
                bs.compile("exit");
                bs.compiling() = 0;
            }, true);
            compile_ext("bye", [](Bootstrap& bs)
            {
                exit(bs.top());
            });
            compile_ext("does>", [](Bootstrap& bs)
            {
            });

            // only debug
            compile_ext("memdump", [](Bootstrap& bs)
            {
                std::string name;
                (*bs.stream) >> name;
                Cell addr = bs.dictionary[name].xt;
                for(int i=0; i < 16; ++i, ++addr)
                    std::cout << "Mem[" << addr << "] = " << bs.mem[addr] << std::endl;
            }, true);

            this->read("\
: state_ptr 0 ; \
: here_ptr 1 ; \
: state state_ptr @ ; \
: here here_ptr @ ; \
: [ immediate 0 state_ptr ! ; \
: ] 1 state_ptr ! ; \
: , here ! here 1 + here_ptr ! ; \
: literal ['] push , , ; \
: if immediate ['] ?branch , here 0 , ; \
: else immediate 0 literal ['] ?branch , here 0 , swap dup here swap - swap ! ; \
: then immediate dup here swap - swap ! ; \
: constant create , does> @ ; \
: ? @ . ; \
: >= < negate ; \
: <= > negate ; \
                       ");
        }

        Cell& compiling(void)
        {
            return this->mem[0];
        }

        Cell& here(void)
        {
            return this->mem[1];
        }

        void read_word(std::string word_str, bool compiling)
        {
            bool immediate = !(compiling || this->compiling());

            // find
            auto it = this->dictionary.find(word_str);
            if(it != this->dictionary.end())
            {
                immediate |= it->second.immediate;
                Cell addr = it->second.xt;
                if(immediate)
                {
                    // execute
                    this->mem[0x11] = addr;
                    this->mem[0x12] = this->dictionary["exit"].xt;
                    this->pc = 0x10;

                    // clean up if external word was called
                    while(!this->return_stack.empty())
                        this->return_stack.pop();

                    this->return_stack.push(0);
                    typename fVM::Program p = typename fVM::Program(*this);
                    giecs::eval(&p);
                }
                else
                    this->compile(addr);
            }
            else
            {
                // number
                try
                {
                    Cell a = std::stoi(word_str);
                    if(immediate)
                        this->push(a);
                    else
                        this->compile_literal(a);
                }
                catch(std::invalid_argument const&)
                {
                    std::cout << "Error: undefined symbol \"" << word_str << "\"!\n";
                }
            }

        }

        bool read_word(void)
        {
            std::string word_str;
            if((*this->stream) >> word_str)
            {
                this->read_word(word_str, false);
                return true;
            }
            return false;
        }

        void read(std::string const& str)
        {
            std::stringstream ss(str);
            this->read(&ss);
        }

        void read(std::istream* stream)
        {
            this->stream = stream;
            while(read_word());
        }

        void repl(std::istream* stream)
        {
            this->stream = stream;
            this->compiling() = 0; // default mode: interpret

            while(1)
            {
                do this->read_word();
                while(this->stream->peek() != '\n');

                std::cout << " ok." << std::endl;
            }
        }
}; // class Bootstrap

} // namespace forth

