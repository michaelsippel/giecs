
#pragma once

#include <cassert>
#include <functional>
#include <boost/unordered_map.hpp>

#include <giecs/memory/accessors/stack.hpp>

namespace giecs
{

template <size_t page_size, typename align_t, typename addr_t>
class Core
{
    public:
        template <typename val_t>
        using Stack = memory::accessors::Stack<page_size, align_t, addr_t, val_t>;

        struct Callback
        {
            virtual void operator()(Stack<align_t>& stack) const {}

            template <typename val_t>
            void operator()(Stack<val_t>& stack) const
            {
                Stack<align_t> s = Stack<align_t>(stack);
                (*this)(s);
                stack.template move<align_t>(s.pos);
            }
        };

        template <typename val_t>
        struct TCallback : public Callback
        {
            TCallback(std::function<void (Stack<val_t>&)> fn_)
                : fn(fn_)
            {
            }

            void operator()(Stack<align_t>& stack) const
            {
                Stack<val_t> s = Stack<val_t>(stack);
                this->fn(s);
                stack.template move<val_t>(s.pos);
            }

            std::function<void (Stack<val_t>&)> fn;
        };

        Core()
        {
            this->operations = boost::unordered_map<addr_t, Callback*>();
        }

        ~Core()
        {
        }

        template <typename val_t>
        void addOperation(addr_t const id, void (*fn)(Stack<val_t>&))
        {
            this->addOperation(id, std::function<void (Stack<val_t>&)>(fn));
        }

        template <typename val_t>
        void addOperation(addr_t const id, std::function<void (Stack<val_t>&)> fn)
        {
            this->operations.insert(std::make_pair(id, new TCallback<val_t>(fn)));
        }

        template <typename val_t>
        void eval(Stack<val_t>& stack) const
        {
            addr_t addr = stack.template pop<addr_t>();

            auto const it = this->operations.find(addr);
            if(it == this->operations.end())
            {
                memory::accessors::Linear<page_size, align_t, addr_t, val_t> abs = memory::accessors::Linear<page_size, align_t, addr_t, val_t>(stack, {0,0});
                size_t len = abs.read(addr);
                assert(len > 0);

                val_t* buf = (val_t*) malloc(len * sizeof(val_t));

                abs.read(++addr, len, buf);
                stack.push(len, buf);

                free(buf);

                this->eval(stack);
            }
            else
            {
                Callback const& fn = *(it->second);
                fn(stack);
            }
        }

    private:
        boost::unordered_map<addr_t, Callback*> operations;
};

} // namespace giecs

