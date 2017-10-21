
#include <iostream>
#include <array>

#include <languages/brainfuck/brainfuck.hpp>

int main(int argc, char* argv[])
{
    using Memory = std::array<char, 4096>;
    using Program = std::array<uint8_t, 4096>;
    using VM = brainfuck::VM<Memory::iterator, Program::iterator>;
    using BFCore = giecs::Core<VM::Instruction, VM::Operator>;

    Memory mem;

    // print all chars
    Program prg;
    Program::iterator p = prg.begin();
    *p++ = (uint8_t)brainfuck::Opcode::inc;
    *p++ = (uint8_t)brainfuck::Opcode::jz;
    *p++ = (uint8_t)4;
    *p++ = (uint8_t)brainfuck::Opcode::inc;
    *p++ = (uint8_t)brainfuck::Opcode::out;
    *p++ = (uint8_t)brainfuck::Opcode::jmp;
    *p++ = (uint8_t)-6;

    auto tape = VM::Instruction::Data(mem.begin(), prg.begin());
    auto code = VM::CodeAccessor(tape.pc, p);
    auto pq = std::queue<VM::Instruction, VM::CodeAccessor>(code);
    BFCore::eval(pq, tape);
}

