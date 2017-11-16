***************************************
    Writing A Brainfuck Interpreter
***************************************

`See full code version <https://github.com/michaelsippel/giecs/tree/dev/languages/brainfuck>`_

The Virtual Machine
===================
The `Complete VM-Implementation <https://github.com/michaelsippel/giecs/tree/dev/languages/brainfuck/vm.hpp>`_ weights ~100 lines of readable C++.

The VM consists of an *Instruction* -type and an *Operator*, which executes the Instructions.
Brainfuck-Instructions are simple, they only need an Opcode (representing **,**, **.**, **+**, **-**, **<**, **>**, **[**, **]**) and an optional data register for jump-instructions.

First lets define our VM-Opcodes with an *enum*:

.. doxygenenum:: brainfuck::Opcode



Now we need to represent the state of the VM. The neccesary values are members of *brainfuck::VM*: Tape and Program pointers have templated types and are expected to behave as iterators on a container (e.g. std::array).

.. doxygenclass:: brainfuck::VM
   :members:


It contains a struct which defines *Instructions*:

.. doxygenstruct:: brainfuck::VM::Instruction
    :undoc-members:

When the *Operator* (defined later) wants to execute an instruction, the method *fetch* is called, which updates the VM-State (in this case load the register) and returns the Opcode to be executed. Note that *Data* is the type for the VM-State.

.. code-block:: c++

    Opcode fetch(Data& data)
    {
        data.jmp_off = this->jmp_off;
        return this->op;
    }


Now we need to implement the opcodes using an *Operator*:

.. code-block:: c++

    #define DATA_FN(def) ([](typename Instruction::Data& d){ def ; })
    #define TAPE_FN(def) DATA_FN( auto& tape = d.tape; def )
        GIECS_CORE_OPERATOR(Operator,
                            ((Opcode::in, TAPE_FN(std::cin >> *tape)))
                            ((Opcode::out, TAPE_FN(std::cout << (char)*tape)))
                            ((Opcode::inc, TAPE_FN(++(*tape))))
                            ((Opcode::dec, TAPE_FN(--(*tape))))
                            ((Opcode::next, TAPE_FN(++tape)))
                            ((Opcode::prev, TAPE_FN(--tape)))
                            ((Opcode::jmp, DATA_FN(_jmp(d))))
                            ((Opcode::jz, TAPE_FN(if(*tape == 0) _jmp(d))))
                           );

This generates a class which can be used with *giecs::eval*.


The last thing to do is to get the *Instructions*. This is done by *CodeAccessor*:

.. doxygenclass:: brainfuck::VM::CodeAccessor
   :members:


Compiling Syntax for the VM
===========================

See `syntax.hpp <https://github.com/michaelsippel/giecs/blob/doc/languages/brainfuck/syntax.hpp>`_

