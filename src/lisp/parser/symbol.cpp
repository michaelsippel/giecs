#include <stddef.h>
#include <string.h>
#include <list.h>

#include <context.h>
#include <lisp/parser.h>

static List<struct symbol*>* symbols = new List<struct symbol*>();
vword_t default_parent = 0;

struct symbol* resolve_symbol(const char* name, vword_t parent)
{
    return resolve_symbol((char*) name, parent);
}

struct symbol* resolve_symbol(char* name)
{
    return resolve_symbol(name, default_parent);
}

struct symbol* resolve_symbol(const char* name)
{
    return resolve_symbol((char*)name, default_parent);
}

struct symbol* resolve_symbol(vword_t addr)
{
    ListIterator<struct symbol*> it = ListIterator<struct symbol*>(symbols);

    while(! it.isLast())
    {
        struct symbol* c = it.getCurrent();
        if(c->start == addr)
            return c;

        it.next();
    }

    return NULL;
}

struct symbol* resolve_symbol(char* name, vword_t parent)
{
    ListIterator<struct symbol*> it = ListIterator<struct symbol*>(symbols);

    while(! it.isLast())
    {
        struct symbol* c = it.getCurrent();
        if(c->name != NULL && strcmp(name, c->name) == 0 && c->parent == parent)
        {
            return c;
        }

        it.next();
    }

    if(parent == 0)
    {
        return NULL;
    }
    else
    {
        parent = resolve_symbol(parent)->parent;
        return resolve_symbol(name, parent);
    }
}

void add_symbol(const char* name, vword_t start)
{
    add_symbol(name, start, 0);
}

void add_symbol(const char* name, vword_t start, size_t reqb)
{
    add_symbol((char*) name, start, reqb);
}

void add_symbol(const char* name, vword_t start, size_t reqb, vword_t parent)
{
    add_symbol((char*) name, start, reqb, parent);
}

void add_symbol(char* name, vword_t start, size_t reqb)
{
    add_symbol(name, start, reqb, default_parent);
}

void add_symbol(char* name, vword_t start, size_t reqb, vword_t parent)
{
    struct symbol* sym = (struct symbol*) malloc(sizeof(struct symbol));

    sym->name = name;
    sym->start = start;
    sym->reqb = reqb;
    sym->parent = parent;

    symbols->pushBack(sym);
}

void remove_symbol(vword_t addr)
{
    struct symbol* s = resolve_symbol(addr);
    symbols->remove(s);
    free(s);
}

void remove_symbol(char* name, vword_t parent)
{
    struct symbol* s = resolve_symbol(name, parent);
    symbols->remove(s);
    free(s);
}

