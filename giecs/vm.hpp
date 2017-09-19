
#pragma once

#include <giecs/memory/context.hpp>
#include <giecs/memory/accessors/linear.hpp>
#include <giecs/memory/accessors/deque.hpp>
#include <giecs/memory/accessors/stack.hpp>
#include <giecs/memory/accessors/queue.hpp>
#include <giecs/core.hpp>

namespace giecs
{

template <std::size_t page_size, typename align_t>
struct VM
{
    using Core = giecs::Core<page_size, align_t>;

    struct Memory
    {
        using Context = giecs::memory::Context<page_size, align_t>;

        struct Accessors
        {
            template <typename addr_t, typename val_t>
            using Linear = giecs::memory::accessors::Linear<page_size, align_t, addr_t, val_t>;

            template <typename addr_t, typename val_t>
            using Deque = giecs::memory::accessors::Deque<page_size, align_t, addr_t, val_t>;

            template <typename val_t>
            using Stack = giecs::memory::accessors::Stack<page_size, align_t, val_t>;

            template <typename val_t>
            using Queue = giecs::memory::accessors::Queue<page_size, align_t, val_t>;
        }; // struct Accessors
    }; // struct Memory
}; // struct VM

} // namespace giecs

