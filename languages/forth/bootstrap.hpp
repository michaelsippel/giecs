
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
            this->dictionary[name] = {immediate, here()};
            this->state[here()] = op;
            ++here();
        }

        void add_comp(std::string name, std::vector<std::string> prg, bool immediate=false)
        {
            this->add_prim(name, Opcode::compose, immediate);
            for(std::string const& word : prg)
            {
                this->state[here()++] = this->dictionary[word].xt;
            }
        }

    public:
        Bootstrap(std::istream& stream_)
            : stream(stream_)
        {
            this->here() = 0x100;

            add_prim("push", Opcode::push);
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

            // Simple extern program
            struct Ext : public giecs::ProgramBase
            {
                    //              giecs::ProgramBase* m_next;

                    struct Evaluator : public giecs::EvaluatorBase
                    {
                        void eval(ProgramBase&)
                        {
                            std::cout << "Test from extern." << std::endl;
                            delete this;
                        }
                    };

                    giecs::EvaluatorBase* createEvaluator(void)
                    {
                        return new Evaluator();
                    }

                    giecs::ProgramBase* next(void)
                    {
                        std::cout << "Next" << std::endl;
                        return nullptr;
                    }
            };

            giecs::ProgramBase* prg = new Ext();
            this->state.programs[789] = prg;
        }

        Cell& compiling(void)
        {
            return this->state[0];
        }

        Cell& here(void)
        {
            return this->state[1];
        }

        void read(std::string& str)
        {
            std::stringstream ss(str);
            this->read(ss);
        }

        void read(std::istream& stream)
        {
            std::string word_str;
            Cell begin = this->here();
            while(stream >> word_str)
            {
                // interpret
                bool compile = this->compiling();

                // find
                auto it = this->dictionary.find(word_str);
                if(it != this->dictionary.end())
                {
                    compile &= !it->second.immediate;
                    this->state[++here()] = it->second.xt;
                }
                else
                {
                    // number
                    try
                    {
                        Cell a = std::stoi(word_str);
                        this->state[++here()] = this->dictionary["push"].xt;
                        this->state[++here()] = a;
                    }
                    catch(std::invalid_argument const&)
                    {
                        std::cout << "Error: undefined symbol \"" << word_str << "\"!\n";
                    }
                }

                if(!compile)
                {
                    // execute
                    this->state[++here()] = this->dictionary["exit"].xt;
                    this->state.pc = begin;
                    this->state.return_stack.push(0);
                    typename fVM::Program p = typename fVM::Program(*this);
                    giecs::eval(&p);

                    this->here() = begin; // FIXME
                }
            }
        }

        void repl(void)
        {
            this->compiling() = 0; // default mode: interpret

            std::string line;
            while(std::getline(this->stream, line))
            {
                this->read(line);
                std::cout << " ok." << std::endl;
            }
        }
}; // class Bootstrap

} // namespace forth

