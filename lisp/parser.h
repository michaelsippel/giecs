
#pragma once

#include <boost/range/adaptor/reversed.hpp>
#include <giecs/memory/accessors/stack.h>

#include <lisp/ast.h>
#include <lisp/context.h>

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
        val_t val(atom());
        context.push(val);
    }
}; // struct Parser< ast::Atom >

template <>
struct Parser< ast::List >
{
    template <int page_size, typename align_t, typename addr_t, typename val_t>
    static void parse(ast::List const& list, Context<page_size, align_t, addr_t, val_t>& context)
    {
        // TODO: should be reversed, but only used for definitions and then called from execution stack, so its okay ;)
//		context.push(list.size());
        for(auto sub : list)
            lisp::parse<page_size, align_t, addr_t, val_t>(sub, context);
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

        case ast::NodeType::string:
            parse_cast<ast::Atom<std::string>, page_size, align_t, addr_t, val_t>(node, context);
            break;
    }
}

} // namespace lisp

