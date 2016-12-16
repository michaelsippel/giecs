# Virtual Memory

## Contexts
The virtual memory of the VM is managed by the *Context*-class.

### Units & Adresses

* **Bit**: smallest unit, has two possible states
* **Byte**: smallest cell in memory, that can be addressed; has a variable size of bits
* **Word**: size needed to address each byte; also typical size for operations

A context can store the number of cells that can be addressed by a *word*.
But the addresses stored in *words* in a *context* are not linear yet!
Think of it as assigning each state of a *word* a memory cell, a *byte*.

### Accessors
Accessors interpret the addresses and cells in a `context` and map the cells to parameters you want.
The simplest form would be linear and 1D.
But you can also map your byte-cells to vectors or non-linear functions.

## Stack Frames
A more advanced method for reading and writing the memory is using stacks. They use a linear, 1D accessor.
This is implemented in the *StackFrame* class which allows to push & pop without worrying about addresses.

