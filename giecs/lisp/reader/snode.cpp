#include <stdio.h>
#include <string.h>

#include <giecs/list.h>
#include <giecs/lisp/reader.h>

SNode::SNode(enum snode_type type_)
    : type(type_)
{
}

SNode::SNode(enum snode_type type_, char* src)
    :type(type_)
{
    this->read(src);
}

SNode::SNode(Context* context, vword_t addr)
{
    this->read_vmem(context, addr);
}

SNode::SNode(StackFrame& stack)
{
    this->pop(stack);
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

void SNode::pop(StackFrame& stack)
{
    this->type = (enum snode_type) stack.pop_word();

    vword_t n;
    int i = 0;
    size_t len;

    vbyte_t buf[512];

    switch(this->type)
    {
        case LIST:
            n = stack.pop_word();
            this->subnodes = new List<SNode*>();
            for(i = 0; i < n; i++)
            {
                SNode* sn = new SNode(stack);
                //sn->dump();
                this->subnodes->pushBack(sn);
            }
            break;

        case SYMBOL:
        case STRING:
            // TODO: arbitrary length
            len = stack.context->read_str(stack.ptr(), buf) + 1;
            this->string = (char*) malloc(len);
            strcpy(this->string, (char*) buf);
            stack.move(-len);
            break;

        case INTEGER:
            this->integer = stack.pop_word();
            break;
    }
}

void SNode::push(StackFrame& stack)
{
    switch(this->type)
    {
        case LIST:
        {
            // insert subnodes
            ListIterator<SNode*> it = ListIterator<SNode*>(this->subnodes);
            it.setLast();
            while(! it.isLast())
            {
                it.getCurrent()->push(stack);
                it.previous();
            }

            // write number of elements
            int n = this->subnodes->numOfElements();
            stack.push_word((vword_t)n);
            break;
        }

        case SYMBOL:
        case STRING:
        {
            int n = strlen(this->string) + 1;
            stack.push((vbyte_t*) this->string, n);
            break;
        }

        case INTEGER:
            stack.push_word((vword_t)this->integer);
            break;
    }

    // write type
    stack.push_word((vword_t)this->type);
}

size_t SNode::vmem_size(void)
{
    size_t len = VWORD_SIZE;

    ListIterator<SNode*>* it;
    switch(this->type)
    {
        case LIST:
            len += VWORD_SIZE;

            // pointers to subnodes
            it = new ListIterator<SNode*>(this->subnodes);
            while(! it->isLast())
            {
                len += it->getCurrent()->vmem_size();
                it->next();
            }
            delete it;
            break;

        case SYMBOL:
        case STRING:
            len += strlen(this->string) + 1;
            break;

        case INTEGER:
            len += VWORD_SIZE;
            break;
    }

    return len;
}

size_t SNode::write_vmem(Context* context, vword_t addr)
{
    size_t len = 0;

    // write type
    context->write_word(addr, (vword_t)this->type);
    len += VWORD_SIZE;

    vword_t n;
    vword_t naddr, maddr;
    ListIterator<SNode*>* it;

    switch(this->type)
    {
        case LIST:
            n = this->subnodes->numOfElements();
            // write number of elements
            context->write_word(addr+len, (vword_t) n);
            len += VWORD_SIZE;

            // insert subnodes
            it = new ListIterator<SNode*>(this->subnodes);
            while(! it->isLast())
            {
                len += it->getCurrent()->write_vmem(context, addr+len);
                it->next();
            }
            delete it;
            break;

        case SYMBOL:
        case STRING:
            n = strlen(this->string) + 1;
            len += context->write(addr+len, n, (vbyte_t*) this->string);
            break;

        case INTEGER:
            context->write_word(addr+len, (vword_t) this->integer);
            len += VWORD_SIZE;
            break;
    }

    return len;
}

size_t SNode::read_vmem(Context* context, vword_t addr)
{
    size_t l = addr;

    this->type = (enum snode_type) context->read_word(addr);
    addr += VWORD_SIZE;

    vword_t n;
    int i = 0;
    size_t len;

    vbyte_t buf[512];

    switch(this->type)
    {
        case LIST:
            n = context->read_word(addr);
            addr += VWORD_SIZE;

            this->subnodes = new List<SNode*>();
            for(i = 0; i < n; i++)
            {
                SNode* sn = new SNode(context, addr);
                addr += sn->vmem_size();
                //sn->dump();
                this->subnodes->pushBack(sn);
            }
            break;

        case SYMBOL:
        case STRING:
            // TODO: arbitrary length
            len = context->read_str(addr, buf) + 1;
            this->string = (char*) malloc(len);
            strcpy(this->string, (char*) buf);
            addr += len;
            break;

        case INTEGER:
            this->integer = context->read_word(addr);
            addr += VWORD_SIZE;
            break;
    }

    l = addr - l;
    return l;
}


void SNode::replace(char** names, SNode** nodes, int num)
{
    if(this->type == SYMBOL)
    {
        int i;
        for(i = 0; i < num; i++)
        {
            if(strcmp(this->string, names[i]) == 0)
            {
                this->type = nodes[i]->type;
                this->integer = nodes[i]->integer;

                return;
            }
        }
    }
    else if(this->type == LIST)
    {
        ListIterator<SNode*> it = ListIterator<SNode*>(this->subnodes);
        while(! it.isLast())
        {
            it.getCurrent()->replace(names, nodes, num);
            it.next();
        }
    }
}

