#ifndef _parser_h_
#define _parser_h_

#include <context.h>
#include <lisp/reader.h>

struct symbol
{
    char* name;
    vword_t start;
};

vword_t resolve_symbol(const char* name);
vword_t resolve_symbol(char* name);
void add_symbol(const char* name, vword_t start);
void add_symbol(char* name, vword_t start);

int parse(Context* context, vword_t addr, SNode* ast);
int parse_se(Context* context, vword_t addr, SNode* ast);
int parse_list(Context* context, vword_t addr, SNode* ast);
int parse_list_se(Context* context, vword_t addr, SNode* ast);
int parse_symbol(Context* context, vword_t addr, SNode* ast);
int parse_string(Context* context, vword_t addr, SNode* ast);
int parse_integer(Context* context, vword_t addr, SNode* ast);

#endif

