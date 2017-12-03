
#include <vector>
#include <iostream>
#include <memory>

#include <giecs/eval.hpp>
#include <languages/forth/bootstrap.hpp>

int main(int argc, char* argv[])
{
    auto vm = forth::Bootstrap<int>();
    vm.repl(&std::cin);
}

