#ifndef _parser_h_
#define _parser_h_

#include <context.h>

struct symbol
{
    char* name;
    vword_t start;
};

vword_t resolve_symbol(Context* parent, const char* name);
vword_t resolve_symbol(Context* parent, char* name);
void add_symbol(const char* name, vword_t start);
void add_symbol(char* name, vword_t start);

#endif

