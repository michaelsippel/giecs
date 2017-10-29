
#include <deque>

#include <languages/forth/vm.hpp>

int main(int argc, char* argv[])
{
    using Word = int;
    using VM = forth::VM<Word, std::deque<Word>>;
    using ForthCore = giecs::Core<VM::Instruction, VM::Operator>;

    VM::Instruction::Data data;
    std::queue<VM::Instruction> prog;

    prog.push({forth::Primitive::push, {2}});
    prog.push({forth::Primitive::push, {3}});
    prog.push({forth::Primitive::addi});

    prog.push({forth::Primitive::dup});
    prog.push({forth::Primitive::printi});

    prog.push({forth::Primitive::dup});
    prog.push({forth::Primitive::muli});
    prog.push({forth::Primitive::printi});

    ForthCore::eval(prog, data);
}

