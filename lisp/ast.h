#pragma once

#include <ostream>
#include <vector>
#include <memory>

namespace lisp
{

namespace ast
{

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

    private:
        virtual void print(std::ostream& stream) const
        {
        }
}; // class Node

class List : public Node
{
    public:
        List() {}

        template <typename N>
        void addNode(std::shared_ptr<N> node)
        {
            this->subnodes.push_back(node);
        }

        template <typename N>
        void addNode(N const& node)
        {
            this->addNode(std::make_shared<N>(node));
        }

    private:
        void print(std::ostream& stream) const final
        {
            stream << "(";
            for(auto it = this->subnodes.begin(); it != this->subnodes.end(); ++it)
            {
                stream << (*(*it));
                if(std::next(it) != this->subnodes.end())
                    stream << " ";
            }
            stream << ")";
        }

        std::vector<std::shared_ptr<Node> > subnodes;
}; // class List

template <typename T>
class Atom : public Node
{
    public:
        Atom() {}
        Atom(T const& value_)
            : value(value_)
        {
        }

    private:
        void print(std::ostream& stream) const final
        {
            stream << this->value;
        }

        T value;
}; // class Atom

} // namespace ast

} // namespace lisp

