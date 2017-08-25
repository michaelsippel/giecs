
#pragma once

#include <vector>
#include <string>
#include <istream>
#include "substreambuf.hpp"

#include <lisp/ast.hpp>

namespace lisp
{

std::shared_ptr<ast::Node> read(ast::NodeType type, std::istream& stream);
std::shared_ptr<ast::Node> read_substream(ast::NodeType type, std::istream& stream, std::streampos start);

template <typename Node>
struct Reader
{
    static std::shared_ptr<Node> read(std::istream& stream)
    {
        return std::make_shared<Node>();
    }
}; // struct Reader

template <typename T>
struct Reader< ast::Atom<T> >
{
    static std::shared_ptr< ast::Atom<T> > read(ast::NodeType type, std::istream& stream)
    {
        T val;
        stream >> val;
        return std::make_shared< ast::Atom<T> >(type, val);
    }
}; // struct Reader<Atom>

template <>
struct Reader< ast::Atom<std::string> >
{
    static std::shared_ptr< ast::Atom<std::string> > read(ast::NodeType type, std::istream& stream)
    {
        std::string val;
        char c;
        while(stream.get(c))
        {
            if(c == '\\')
            {
                stream.get(c);
                switch(c)
                {
                    case 'n':
                        c = '\n';
                        break;

                    case 't':
                        c = '\t';
                        break;

                    case 'v':
                        c = '\v';
                        break;

                    case 'b':
                        c = '\b';
                        break;

                    case 'r':
                        c = '\r';
                        break;

                    case 'f':
                        c = '\f';
                        break;

                    case 'a':
                        c = '\a';
                        break;

                    case '\\':
                        c = '\\';
                        break;

                    case '?':
                        c = '\?';
                        break;

                    case '\'':
                        c = '\'';
                        break;

                    case '\"':
                        c = '\"';
                        break;
                }
            }
            val += c;
        }
        return std::make_shared< ast::Atom<std::string> >(type, val);
    }
}; // struct Reader<Atom<std::string> >

template <>
struct Reader<ast::List>
{
    static std::shared_ptr<ast::List> read(std::istream& stream)
    {
        std::shared_ptr<ast::List> list = std::make_shared<ast::List>();

        bool string = false;
        bool comment = false;
        bool quote = false;
        int depth = 0;
        std::streampos start;

        ast::NodeType st = ast::NodeType::none;

        char c;
        while(stream.get(c))
        {
            if(st == ast::NodeType::none && !comment)
            {
                switch(c)
                {
                    case ')':
                        std::cout << "error: unexpected )" << std::endl;
                        break;

                    case '(':
                        st = ast::NodeType::list;
                        stream.get(c);
                        depth++;
                        break;

                    case '"':
                        st = ast::NodeType::string;
                        string = true;
                        stream.get(c);
                        break;

                    case '\'':
                        quote = true;
                        break;

                    case ';':
                    case '#':
                        comment = true;

                    case '\n':
                    case ' ':
                    case '\t':
                        break;

                    default:
                        if((c >= '0' && c <= '9') ||
                                (c == '-' && stream.peek() >= '0' && stream.peek() <= '9'))
                            st = ast::NodeType::integer;
                        else
                            st = ast::NodeType::symbol;
                        break;
                }
                start = stream.tellg()-1;
            }

            if(st != ast::NodeType::none && !comment)
            {
                switch(c)
                {
                    case '(':
                        ++depth;
                        break;

                    case ')':
                        --depth;
                        if(!string)
                            goto add;

                        break;

                    case '"':
                        if(string)
                            goto add;
                        else if(depth == 0)
                        {
                            std::cout << "error: unexpexted \"" << std::endl;
                        }

                        break;

                    case ';':
                    case '#':
                        if(!string)
                            comment = true;
                        else
                            break;

                    case '\n':
                    case '\t':
                    case '\0':
                    case ' ':
                        if(!string)
                            goto add;

                    default:
                        break;
add:
                        if(depth == 0)
                        {
add_last:
                            std::shared_ptr<ast::Node> sn;
                            if(quote)
                            {
                                std::shared_ptr<ast::List> sl = std::make_shared<ast::List>();
                                sl->addNode(ast::Atom<std::string>(ast::NodeType::symbol, "quote"));
                                sl->addNode(read_substream(st, stream, start));

                                sn = sl;
                            }
                            else
                                sn = read_substream(st, stream, start);

                            list->addNode(sn);
                            string = false;
                            quote = false;
                            st = ast::NodeType::none;
                        }
                }
            }
            else if(comment)
            {
                if(c == '\n')
                    comment = false;
            }
        }

        if(st != ast::NodeType::none)
            goto add_last;

        return list;
    }
}; // struct Reader<List>

std::shared_ptr<ast::Node> read(ast::NodeType type, std::istream& stream)
{
    switch(type)
    {
        case ast::NodeType::list:
            return Reader<ast::List>::read(stream);

        case ast::NodeType::integer:
            return Reader<ast::Atom<int>>::read(type, stream);

        case ast::NodeType::symbol:
        case ast::NodeType::string:
            return Reader<ast::Atom<std::string>>::read(type, stream);
    }
}

std::shared_ptr<ast::Node> read_substream(ast::NodeType type, std::istream& stream, std::streampos start)
{
    std::streampos end;
    if(stream.eof())
    {
        stream.clear();
        end = stream.tellg()+1;
    }
    else
        end = stream.tellg();

    Substreambuf ssb(stream.rdbuf(), start, end-start);
    std::istream substream(&ssb);

    return read(type, substream);
}

} // namespace lisp

