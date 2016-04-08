#include <stddef.h>
#include <string.h>
#include <list.h>

#include <context.h>
#include <lisp/parser.h>

static List<struct symbol>* symbols = new List<struct symbol>();

struct symbol resolve_symbol(const char* name)
{
    return resolve_symbol((char*) name);
}

struct symbol resolve_symbol(char* name)
{
    ListIterator<struct symbol> it = ListIterator<struct symbol>(symbols);

    while(! it.isLast())
    {
        struct symbol c = it.getCurrent();
        if(strcmp(name, c.name) == 0)
        {
            return c;
        }

        it.next();
    }

    return (struct symbol)
    {
        NULL, 0, 0
    };
}

void add_symbol(const char* name, vword_t start)
{
    add_symbol(name, start, 0);
}

void add_symbol(const char* name, vword_t start, size_t reqb)
{
    add_symbol((char*) name, start, reqb);
}

void add_symbol(char* name, vword_t start, size_t reqb)
{
    symbols->pushBack({name, start, reqb});
}

