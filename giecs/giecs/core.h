
#pragma once

#include <boost/unordered_map.hpp>

#include <giecs/memory/accessors/stack.h>

namespace giecs
{

template <size_t page_size, typename align_t, typename addr_t, typename val_t>
class Core
{
    public:
        typedef memory::accessors::Stack<page_size, align_t, addr_t, val_t> Stack;
        typedef void (*Callback)(Stack&); // eventually boost::function

        Core()
        {
            this->operations = boost::unordered_map<addr_t, Callback>();
        }

        ~Core()
        {
        }

        void addOperation(addr_t const id, Callback const fn)
        {
            this->operations.insert(std::make_pair(id, fn));
        }

        void eval(Stack& stack) const
        {
            addr_t addr = stack.pop();

            auto it = this->operations.find(addr);
            if(it == this->operations.end())
            {
                int len = stack.read(addr);
                val_t* buf = (val_t*) malloc(len * sizeof(val_t));

                stack.read(++addr, len, buf);
                stack.push(len, buf);

                free(buf);

                this->eval(stack);
            }
            else
            {
                Callback fn = it->second;
                fn(stack);
            }
        }

    private:
        boost::unordered_map<addr_t, Callback> operations;
};

}; // namespace giecs

