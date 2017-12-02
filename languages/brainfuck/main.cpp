
#include <iostream>
#include <array>
#include <deque>
#include <fstream>

#include <languages/brainfuck/vm.hpp>
#include <languages/brainfuck/syntax.hpp>

int main(int argc, char* argv[])
{
    using Memory = std::array<short, 30000>;
    using Program = std::array<uint8_t, 30000>;
    using VM = brainfuck::VM<Memory::iterator, Program::iterator>;
    using BFCore = giecs::Core<VM::Instruction, VM::Operator>;

    if(argc > 1)
    {
        std::ifstream file(argv[1]);

        brainfuck::SyntaxAccessor<std::deque<uint8_t>> reader;
        reader.parse(file);

        Program prg;
        Program::iterator p = prg.begin();
        for(int i=0; !reader.empty(); ++i)
        {
            *p++ = (uint8_t) reader.front();
            reader.pop();
        }

        Memory mem = {0};
        VM::Instruction::Data state = {mem.begin(), prg.begin()};
        auto code = VM::CodeAccessor(state.pc, p);
        auto pq = std::queue<VM::Instruction, VM::CodeAccessor>(code);
        BFCore::eval(pq, state);
    }
}

