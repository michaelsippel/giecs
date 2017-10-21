#pragma once

#include <ostream>
#include <vector>
#include <memory>

namespace lisp
{

namespace ast
{

enum NodeType
{
    none,
    list,
    symbol,
    integer,
    string,
};


class Node
{
    public:
        virtual ~Node()
        {
        }

        friend std::ostream& operator<< (std::ostream& stream, Node const& node)
        {
            node.print(stream);
            return stream;
        }

        virtual NodeType getType(void) const
        {
            return NodeType::none;
        }

    private:
        virtual void print(std::ostream& stream) const
        {
        }
}; // class Node

template <typename T>
class Atom : public Node
{
    public:
        Atom() {}
        Atom(NodeType type_, T const& value_)
            : type(type_), value(value_)
        {
        }

        T operator() (void) const
        {
            return this->value;
        }

        virtual NodeType getType(void) const
        {
            return this->type;
        }

    private:
        void print(std::ostream& stream) const final
        {
            if(type == NodeType::string)
                stream << "\"" << this->value << "\"";
            else
                stream << this->value;
        }

        NodeType type;
        T value;
}; // class Atom

class List : public Node, public std::vector< std::shared_ptr<Node> >
{
    public:
        List() {}

        template <typename N>
        void addNode(std::shared_ptr<N> node)
        {
            this->push_back(node);
        }

        template <typename N>
        void addNode(N const& node)
        {
            this->addNode(std::make_shared<N>(node));
        }

        NodeType getType(void) const
        {
            return NodeType::list;
        }

        void replace_symbol(std::string a, std::shared_ptr<ast::Node> b)
        {
            for(auto it = this->begin(); it != this->end(); ++it)
            {
                switch((*it)->getType())
                {
                    case NodeType::list:
                        std::static_pointer_cast<List>(*it)->replace_symbol(a, b);
                        break;

                    case NodeType::symbol:
                        if((*std::static_pointer_cast<Atom<std::string>>(*it))() == a)
                        {
                            it = this->erase(it);
                            it = this->insert(it, b);
                        }
                        break;

                    default:
                        break;
                }
            }
        }

    private:
        void print(std::ostream& stream) const final
        {
            stream << "(";
            for(auto it = this->begin(); it != this->end(); ++it)
            {
                stream << (*(*it));
                if(std::next(it) != this->end())
                    stream << " ";
            }
            stream << ")";
        }
}; // class List

} // namespace ast

} // namespace lisp

