#include <stddef.h>
#include <string.h>
#include <list.h>

#include <context.h>
#include <lisp/parser.h>

Namespace::Namespace(Namespace* parent_)
    : parent(parent_)
{
    this->symbols = new List<struct symbol*>();
}

Namespace::~Namespace()
{
    ListIterator<struct symbol*> it = ListIterator<struct symbol*>(symbols);
    while(! it.isLast())
    {
        free(it.getCurrent());
        it.next();
    }

    delete this->symbols;
}

struct symbol* Namespace::resolve_symbol(const char* name)
{
    return this->resolve_symbol((char*)name);
}

struct symbol* Namespace::resolve_symbol(vword_t addr)
{
    ListIterator<struct symbol*> it = ListIterator<struct symbol*>(this->symbols);

    while(! it.isLast())
    {
        struct symbol* c = it.getCurrent();
        if(c->start == addr)
            return c;

        it.next();
    }

    if(this->parent != NULL)
        return this->parent->resolve_symbol(addr);

    return NULL;
}

struct symbol* Namespace::resolve_symbol(char* name)
{
    ListIterator<struct symbol*> it = ListIterator<struct symbol*>(this->symbols);
    while(! it.isLast())
    {
        struct symbol* c = it.getCurrent();
        if(c->name != NULL)
        {
            if(strcmp(name, c->name) == 0)
            {
                return c;
            }
        }

        it.next();
    }

    if(this->parent != NULL)
        return this->parent->resolve_symbol(name);

    return NULL;
}

void Namespace::add_symbol(const char* name, vword_t start)
{
    this->add_symbol(name, start, 0);
}

void Namespace::add_symbol(const char* name, vword_t start, size_t reqb)
{
    this->add_symbol((char*) name, start, reqb);
}

void Namespace::add_symbol(char* name, vword_t start, size_t reqb)
{
    struct symbol* sym = (struct symbol*) malloc(sizeof(struct symbol));

    sym->name = name;
    sym->start = start;
    sym->reqb = reqb;

    symbols->pushFront(sym);
}

void Namespace::remove_symbol(vword_t addr)
{
    struct symbol* s = resolve_symbol(addr);
    this->symbols->remove(s);
    free(s);
}

void Namespace::remove_symbol(char* name)
{
    struct symbol* s = resolve_symbol(name);
    this->symbols->remove(s);
    free(s);
}


// default namespace
Namespace* default_namespace;

struct symbol* resolve_symbol(const char* name)
{
    return default_namespace->resolve_symbol(name);
}

struct symbol* resolve_symbol(char* name)
{
    return default_namespace->resolve_symbol(name);
}

struct symbol* resolve_symbol(vword_t addr)
{
    return default_namespace->resolve_symbol(addr);
}

void add_symbol(const char* name, vword_t start)
{
    default_namespace->add_symbol(name, start);
}

void add_symbol(const char* name, vword_t start, size_t reqb)
{
    default_namespace->add_symbol(name, start, reqb);
}

void add_symbol(char* name, vword_t start, size_t reqb)
{
    default_namespace->add_symbol(name, start, reqb);
}

void remove_symbol(vword_t addr)
{
    default_namespace->remove_symbol(addr);
}

void remove_symbol(char* name)
{
    default_namespace->remove_symbol(name);
}

