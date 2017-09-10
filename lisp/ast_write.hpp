#pragma once

#include <memory>
#include <cstddef>

#include <giecs/memory/accessors/stack.hpp>
#include <lisp/ast.hpp>

namespace lisp
{

template <int page_size, typename align_t, typename addr_t, typename val_t>
std::size_t write_ast(std::shared_ptr<ast::Atom<int>> atom, giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
{
    stack.push(ast::NodeType::integer);
    stack.push((*atom)());

    return 2;
}

template <int page_size, typename align_t, typename addr_t, typename val_t>
std::size_t write_ast(std::shared_ptr<ast::Atom<std::string>> atom, giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
{
    stack.push(atom->getType());
    int len = (*atom)().length();
    stack.push(val_t(len));
    for(char a : (*atom)())
        stack.push(val_t(a));

    return (2+len);
}

template <int page_size, typename align_t, typename addr_t, typename val_t>
std::size_t write_ast(std::shared_ptr<ast::List> list, giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
{
    stack.push(ast::NodeType::list);
    stack.push(val_t(list->size()));

    size_t len = 2;
    for(auto sub : *list)
    {
        switch(sub->getType())
        {
            case ast::NodeType::list:
                len += write_ast<page_size, align_t, addr_t, val_t>(std::static_pointer_cast<ast::List>(sub), stack);
                break;

            case ast::NodeType::integer:
                len += write_ast<page_size, align_t, addr_t, val_t>(std::static_pointer_cast<ast::Atom<int>>(sub), stack);
                break;

            case ast::NodeType::string:
            case ast::NodeType::symbol:
                len += write_ast<page_size, align_t, addr_t, val_t>(std::static_pointer_cast<ast::Atom<std::string>>(sub), stack);
                break;

            default:
                break;
        }
    }

    return len;
}

template <int page_size, typename align_t, typename addr_t, typename val_t>
std::size_t write_ast(std::shared_ptr<ast::Node> node, giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
{
    switch(node->getType())
    {
        case ast::NodeType::list:
            return write_ast<page_size, align_t, addr_t, val_t>(std::static_pointer_cast<ast::List>(node), stack);

        case ast::NodeType::integer:
            return write_ast<page_size, align_t, addr_t, val_t>(std::static_pointer_cast<ast::Atom<int>>(node), stack);

        case ast::NodeType::symbol:
        case ast::NodeType::string:
            return write_ast<page_size, align_t, addr_t, val_t>(std::static_pointer_cast<ast::Atom<std::string>>(node), stack);

        default:
            return 0;
    }
}

template <int page_size, typename align_t, typename addr_t, typename val_t>
std::shared_ptr<ast::Node> read_ast(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
{
    ast::NodeType type = stack.pop();
    int len = stack.pop();

    switch(type)
    {
        case ast::NodeType::list:
        {
            std::shared_ptr<ast::List> list = std::make_shared<ast::List>();
            for(int i = 0; i < len; ++i)
                list->addNode(read_ast<page_size, align_t, addr_t, val_t>(stack));
            return list;
        }

        case ast::NodeType::integer:
            return std::make_shared<ast::Atom<int>>(ast::NodeType::integer, len);

        case ast::NodeType::string:
        case ast::NodeType::symbol:
        {
            std::string name;
            for(int i = 0; i < len; ++i)
                name += char(stack.pop());
            return std::make_shared<ast::Atom<std::string>>(type, name);
        }
        default:
            return nullptr;
    }
}

} // namespace lisp

