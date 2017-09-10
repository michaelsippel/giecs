
#pragma once

#include <boost/range/adaptor/reversed.hpp>
#include <giecs/memory/accessors/stack.hpp>

#include <lisp/ast.hpp>
#include <lisp/ast_write.hpp>
#include <lisp/context.hpp>

namespace lisp
{

template <int page_size, typename align_t, typename addr_t, typename val_t>
void parse(std::shared_ptr<ast::Node> node, Context<page_size, align_t, addr_t, val_t>& context);

template <typename T>
struct Parser
{
    template <int page_size, typename align_t, typename addr_t, typename val_t>
    static void parse(T const& a, Context<page_size, align_t, addr_t, val_t>& stack)
    {
    }
}; // struct Parser

template <typename T>
struct Parser< ast::Atom<T> >
{
    template <int page_size, typename align_t, typename addr_t, typename val_t>
    static void parse(ast::Atom<T> const& atom, Context<page_size, align_t, addr_t, val_t>& context)
    {
        context.push(atom());
    }
}; // struct Parser< ast::Atom >

template <>
struct Parser< ast::List >
{
    template <int page_size, typename align_t, typename addr_t, typename val_t>
    static void parse(ast::List const& list, Context<page_size, align_t, addr_t, val_t>& context)
    {
        addr_t laddr = context.def_ptr();
        context.push(val_t());

        auto it = list.begin();
        lisp::parse<page_size, align_t, addr_t, val_t>(*it, context);
        ++it;

        size_t len = 1;
        for(; it != list.end(); ++it)
            len += context.push_ast(*it);

        context.write_def(laddr, len);
    }
}; // struct Parser< ast::List >

template <typename T, int page_size, typename align_t, typename addr_t, typename val_t>
void parse_cast(std::shared_ptr<ast::Node> node, Context<page_size, align_t, addr_t, val_t>& context)
{
    Parser<T>::template parse<page_size, align_t, addr_t, val_t>(*std::static_pointer_cast<T>(node), context);
}

template <int page_size, typename align_t, typename addr_t, typename val_t>
void parse(std::shared_ptr<ast::Node> node, Context<page_size, align_t, addr_t, val_t>& context)
{
    switch(node->getType())
    {
        case ast::NodeType::list:
            parse_cast<ast::List, page_size, align_t, addr_t, val_t>(node, context);
            break;

        case ast::NodeType::integer:
            parse_cast<ast::Atom<int>, page_size, align_t, addr_t, val_t>(node, context);
            break;

        case ast::NodeType::symbol:
            parse_cast<ast::Atom<std::string>, page_size, align_t, addr_t, val_t>(node, context);
            break;

        default:
            break;
    }
}

template <int page_size, typename align_t, typename addr_t, typename val_t>
void asm_parse(ast::List const& list, Context<page_size, align_t, addr_t, val_t>& context)
{
    auto l1 = context.create_list(list.size());
    for(auto sub : list)
    {
        switch(sub->getType())
        {
            case ast::NodeType::list:
                l1.push_back(context.def_ptr());
                asm_parse(*std::static_pointer_cast<ast::List>(sub), context);
                break;

            case ast::NodeType::integer:
                l1.push_back((*std::static_pointer_cast<ast::Atom<int>>(sub))());
                break;

            case ast::NodeType::symbol:
                l1.push_back(context.resolve_symbol((*std::static_pointer_cast<ast::Atom<std::string>>(sub))()));
                break;

            default:
                break;
        }
    }

    context.push(l1);
}

} // namespace lisp

