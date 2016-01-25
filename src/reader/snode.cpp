#include <stdio.h>
#include <string.h>

#include <list.h>
#include <reader.h>

SNode::SNode(enum snode_type type_)
    : type(type_)
{
}

SNode::SNode(enum snode_type type_, char* src)
    :type(type_)
{
    this->read(src);
}

SNode::~SNode()
{
    // TODO
}


void SNode::read(char* src)
{
    this->read(src, src + strlen(src));
}

void SNode::read(char* start, char* end)
{
    switch(this->type)
    {
        case LIST:
            this->read_list(start, end);
            break;

        case SYMBOL:
            this->read_symbol(start, end);
            break;

        case STRING:
            this->read_string(start, end);
            break;

        case INTEGER:
            this->read_integer(start, end);
            break;
    }
}

void SNode::dump(void)
{
    this->dump(0);
}

void SNode::dump(int indent)
{
    int i;
    for(i = 0; i < indent; i++)
        printf("\t");

    ListIterator<SNode*>* it;
    switch(this->type)
    {
        case LIST:
            it = new ListIterator<SNode*>(this->subnodes);
            printf("List, %d entries:\n", this->subnodes->numOfElements());
            while(! it->isLast())
            {
                it->getCurrent()->dump(indent+1);
                it->next();
            }
            delete it;
            break;

        case SYMBOL:
            printf("Symbol: %s\n", this->string);
            break;

        case STRING:
            printf("String: \"%s\"\n", this->string);
            break;

        case INTEGER:
            printf("Integer: %d\n", this->integer);
            break;

    }
}

