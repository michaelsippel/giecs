#include <stddef.h>
#include <string.h>
#include <list.h>

#include <context.h>
#include <lisp/parser.h>

Namespace::Namespace(Namespace* parent_)
    : parent(parent_)
{
    this->symbols = new List<struct symbol*>();
    this->parselist = new List<struct parsepoint>();
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
    delete this->parselist;
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

size_t Namespace::get_reqb(vword_t addr)
{
    ListIterator<struct parsepoint> it = ListIterator<struct parsepoint>(this->parselist);

    while(! it.isLast())
    {
        struct parsepoint pp = it.getCurrent();
        if(pp.addr == addr)
            return pp.reqb;
        it.next();
    }

    if(this->parent != NULL)
        return this->parent->get_reqb(addr);

    return 0;
}

void Namespace::add_parsepoint(vword_t addr, size_t reqb)
{
    this->parselist->pushBack((struct parsepoint)
    {
        addr, reqb
    });
}

void Namespace::remove_parsepoint(vword_t addr)
{
    ListIterator<struct parsepoint> it = ListIterator<struct parsepoint>(this->parselist);

    while(! it.isLast())
    {
        struct parsepoint pp = it.getCurrent();
        if(pp.addr == addr)
        {
            it.remove();
            return;
        }
        it.next();
    }

    if(this->parent != NULL)
        return this->parent->remove_parsepoint(addr);
}

void Namespace::add_symbol(const char* name, vword_t start)
{
    this->add_symbol((char*)name, start);
}

void Namespace::add_symbol(const char* name, vword_t start, size_t reqb)
{
    this->add_symbol((char*) name, start, reqb);
}

void Namespace::add_symbol(const char* name, vword_t start, size_t reqb, Namespace* ns)
{
    this->add_symbol((char*) name, start, reqb, ns);
}

void Namespace::add_symbol(char* name, vword_t start)
{
    this->add_symbol(name, start, 0);
}

void Namespace::add_symbol(char* name, vword_t start, size_t reqb)
{
    this->add_symbol(name, start, reqb, new Namespace(this));
}

void Namespace::add_symbol(char* name, vword_t start, size_t reqb, Namespace* ns)
{
    struct symbol* sym = (struct symbol*) malloc(sizeof(struct symbol));

    sym->name = name;
    sym->start = start;
    sym->ns = ns;

    symbols->pushFront(sym);

    if(reqb > 0)
        this->add_parsepoint(start, reqb);
}

void Namespace::remove_symbol(vword_t addr)
{
    struct symbol* s = resolve_symbol(addr);
    delete s->ns;
    this->remove_parsepoint(s->start);
    this->symbols->remove(s);
    free(s);
}

void Namespace::remove_symbol(char* name)
{
    struct symbol* s = resolve_symbol(name);
    delete s->ns;
    this->remove_parsepoint(s->start);
    this->symbols->remove(s);
    free(s);
}


// default namespace
Namespace* default_namespace;


void add_parsepoint(vword_t addr, size_t reqb)
{
    default_namespace->add_parsepoint(addr, reqb);
}

void remove_parsepoint(vword_t addr)
{
    default_namespace->remove_parsepoint(addr);
}

size_t get_reqb(vword_t addr)
{
    return default_namespace->get_reqb(addr);
}

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

void add_symbol(const char* name, vword_t start, size_t reqb, Namespace* ns)
{
    default_namespace->add_symbol(name, start, reqb, ns);
}

void add_symbol(char* name, vword_t start)
{
    default_namespace->add_symbol(name, start);
}

void add_symbol(char* name, vword_t start, size_t reqb)
{
    default_namespace->add_symbol(name, start, reqb);
}

void add_symbol(char* name, vword_t start, size_t reqb, Namespace* ns)
{
    default_namespace->add_symbol(name, start, reqb, ns);
}

void remove_symbol(vword_t addr)
{
    default_namespace->remove_symbol(addr);
}

void remove_symbol(char* name)
{
    default_namespace->remove_symbol(name);
}

