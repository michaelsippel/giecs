#include <stddef.h>
#include <string.h>
#include <list.h>

#include <context.h>
#include <parser.h>

static List<struct symbol>* symbols = new List<struct symbol>();

vword_t resolve_symbol(const char* name)
{
    return resolve_symbol((char*) name);
}

vword_t resolve_symbol(char* name)
{
    ListIterator<struct symbol> it = ListIterator<struct symbol>(symbols);

    while(! it.isLast())
    {
        struct symbol c = it.getCurrent();
        if(strcmp(name, c.name) == 0)
        {
            return c.start;
        }

        it.next();
    }

    return 0;
}

void add_symbol(const char* name, vword_t start)
{
    add_symbol((char*) name, start);
}

void add_symbol(char* name, vword_t start)
{
    symbols->pushBack({name, start});
}

