
#pragma once

#include <lisp/ast.h>

namespace lisp
{

void parse(std::shared_ptr<ast::Node> node);

template <typename T>
struct Parser
{
    static void parse(T const& a)
    {
    }
}; // struct Parser

template <typename T>
struct Parser< ast::Atom<T> >
{
    static void parse(ast::Atom<T> const& atom)
    {
        std::cout << "parse atom " << atom << std::endl;
    }
}; // struct Parser< ast::Atom >

template <>
struct Parser< ast::List >
{
    static void parse(ast::List const& list)
    {
        std::cout << "parse list " << list << std::endl;
        for(auto sub : list)
            ::lisp::parse(sub);
    }
}; // struct Parser< ast::List >

template <typename T>
void parse_cast(std::shared_ptr<ast::Node> node)
{
    Parser<T>::parse(*std::static_pointer_cast<T>(node));
}

void parse(std::shared_ptr<ast::Node> node)
{
    switch(node->getType())
    {
        case ast::NodeType::list:
            parse_cast<ast::List>(node);
            break;

        case ast::NodeType::integer:
            parse_cast<ast::Atom<int>>(node);
            break;

        case ast::NodeType::symbol:
        case ast::NodeType::string:
            parse_cast<ast::Atom<std::string>>(node);
            break;
    }
}

} // namespace lisp

