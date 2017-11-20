
#include <vector>
#include <iostream>
#include <memory>

#include <giecs/eval.hpp>
#include <languages/forth/bootstrap.hpp>

int main(int argc, char* argv[])
{
    auto vm = forth::Bootstrap<int>(std::cin);
    /*
    using Opcode = forth::Bootstrap<int>::Opcode;
    vm.state[51] = 200;
    vm.state[52] = 4;
    vm.state[53] = 100;
    vm.state[54] = 500;
    vm.state[55] = 600;

    vm.state[100] = Opcode::compose;
    vm.state[101] = 300;
    vm.state[102] = 400;
    vm.state[103] = 600;

    vm.state[200] = Opcode::push;
    vm.state[300] = Opcode::dup;
    vm.state[400] = Opcode::muli;
    vm.state[500] = Opcode::printi;
    vm.state[600] = Opcode::exit;

    vm.state.return_stack.push(0);
    vm.state.pc = 50;
    auto p = typename forth::Bootstrap<int>::Program(vm);
    giecs::eval(&p);
    std::cout << std::endl;
    */
    vm.repl();
}

