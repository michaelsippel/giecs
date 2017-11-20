
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
class Bootstrap : public VM<Cell, std::vector<Cell>, std::vector<Cell>, std::array<Cell, 10000>>
{
    public:
        using fVM = VM<Cell, std::vector<Cell>, std::vector<Cell>, std::array<Cell, 10000>>;
        using Instruction = typename fVM::Instruction;
        using Opcode = typename Instruction::Opcode;

        std::istream& stream;
        struct Entry
        {
            bool immediate;
            Cell xt;
        };

        std::map<std::string, Entry> dictionary;

        void add_prim(std::string name, Opcode op, bool immediate=false)
        {
            Cell addr = here()++;
            this->dictionary[name] = {immediate, addr};
            this->state[addr] = op;
        }

        void add_comp(std::string name, std::vector<std::string> prg, bool immediate=false)
        {
            this->add_prim(name, Opcode::compose, immediate);
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
        void add_ext(std::string name, Functor const& fn, bool immediate=false)
        {
            giecs::ProgramBase* prg = new Ext<Functor>(fn, *this);
            Cell addr = here()++;
            this->state.programs[addr] = prg;
            this->add_prim(name, Opcode::compose, immediate);
            this->state[here()++] = this->dictionary["push"].xt;
            this->state[here()++] = addr;
            this->state[here()++] = this->dictionary["execute"].xt;
        }

    public:
        Bootstrap(std::istream& stream_)
            : stream(stream_)
        {
            this->here() = 0x100;

            add_prim("push", Opcode::push);
            add_prim("?branch", Opcode::branch);
            add_prim("exit", Opcode::exit);
            add_prim("!", Opcode::store);
            add_prim("@", Opcode::load);
            add_prim("dup", Opcode::dup);
            add_prim("over", Opcode::over);
            add_prim("drop", Opcode::drop);
            add_prim("swap", Opcode::swap);
            add_prim(">r", Opcode::pushr);
            add_prim("r>", Opcode::popr);

            add_prim("not", Opcode::noti);
            add_prim("and", Opcode::andi);
            add_prim("or", Opcode::ori);
            add_prim("xor", Opcode::xori);
            add_prim("+", Opcode::addi);
            add_prim("-", Opcode::subi);
            add_prim("*", Opcode::muli);
            add_prim("/", Opcode::divi);
            add_prim("=", Opcode::eq);

            add_prim("emit", Opcode::emit);
            add_prim(".", Opcode::printi);

            add_comp("?", {"@", ".", "exit"});
            add_comp("execute", {">r", "exit"});
            add_comp("state", {"0", "@", "exit"});
            add_comp("[", {"0", "state", "!", "exit"}, true);
            add_comp("]", {"1", "state", "!", "exit"}, true);
            //add_comp("constant", {"create", "does>", "@", "exit"}, true);

            add_ext("'", [](Bootstrap& bs)
            {
                std::string name;
                bs.stream >> name;
                bs.state.push(bs.dictionary[name].xt);
            }, true);

            add_ext(":", [](Bootstrap& bs)
            {
                std::string name;
                bs.stream >> name;
                std::cout << "DEFINE " << name << std::endl;
                bs.add_prim(name, Opcode::compose);
                bs.compiling() = 1;
            }, true);

            add_ext(";", [](Bootstrap& bs)
            {
                bs.read_word("exit", true);
                bs.compiling() = 0;
            }, true);

            add_ext("bye", [](Bootstrap& bs)
            {
                exit(bs.state.top());
            });
        }

        Cell& compiling(void)
        {
            return this->state[0];
        }

        Cell& here(void)
        {
            return this->state[1];
        }

        void read_word(std::string word_str, bool compiling)
        {
            // std::cout << "Compile: " << word_str << std::endl;
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
                    this->state[0x11] = addr;
                    this->state[0x12] = this->dictionary["exit"].xt;
                    this->state.pc = 0x10;

                    // clean up if external word was called
                    while(!this->state.return_stack.empty())
                        this->state.return_stack.pop();

                    this->state.return_stack.push(0);
                    typename fVM::Program p = typename fVM::Program(*this);
                    giecs::eval(&p);
                }
                else
                    this->state[here()++] = addr;
            }
            else
            {
                // number
                try
                {
                    Cell a = std::stoi(word_str);
                    if(immediate)
                        this->state.push(a);
                    else
                    {
                        this->state[here()++] = this->dictionary["push"].xt;
                        this->state[here()++] = a;
                    }
                }
                catch(std::invalid_argument const&)
                {
                    std::cout << "Error: undefined symbol \"" << word_str << "\"!\n";
                }
            }

        }

        void read(std::string& str)
        {
            std::stringstream ss(str);
            this->read(ss);
        }

        void read(std::istream& stream)
        {
            std::string word_str;
            stream >> word_str;
            this->read_word(word_str, false);
        }

        void repl(void)
        {
            this->compiling() = 0; // default mode: interpret

            while(1)
            {
                do this->read(this->stream);
                while(this->stream.peek()!='\n');

                std::cout << " ok." << std::endl;
            }
        }
}; // class Bootstrap

} // namespace forth

