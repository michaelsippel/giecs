#include <stddef.h>
#include <string.h>
#include <list.h>

#include <context.h>
#include <lisp/parser.h>

static List<struct symbol>* symbols = new List<struct symbol>();

struct symbol resolve_symbol(const char* name, vword_t parent)
{
    return resolve_symbol((char*) name, parent);
}

struct symbol resolve_symbol(char* name)
{
    return resolve_symbol(name, 0);
}

struct symbol resolve_symbol(const char* name)
{
    return resolve_symbol((char*)name, 0);
}

struct symbol resolve_symbol(vword_t addr)
{
    ListIterator<struct symbol> it = ListIterator<struct symbol>(symbols);

    while(! it.isLast())
    {
        struct symbol c = it.getCurrent();
        if(c.start == addr)
            return c;

        it.next();
    }

    return (struct symbol)
    {
        NULL, 0, 0, 0
    };
}

struct symbol resolve_symbol(char* name, vword_t parent)
{
    ListIterator<struct symbol> it = ListIterator<struct symbol>(symbols);

    while(! it.isLast())
    {
        struct symbol c = it.getCurrent();
        if(strcmp(name, c.name) == 0 && c.parent == parent)
        {
            return c;
        }

        it.next();
    }

    if(parent == 0)
    {
        return (struct symbol)
        {
            NULL, 0, 0, 0
        };
    }
    else
    {
        parent = resolve_symbol(parent).parent;
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
    add_symbol(name, start, reqb, 0);
}

void add_symbol(char* name, vword_t start, size_t reqb, vword_t parent)
{
    symbols->pushBack({name, start, reqb, parent});
}

