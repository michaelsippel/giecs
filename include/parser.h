#ifndef _parser_h_
#define _parser_h_

#include <context.h>
#include <reader.h>

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

void init_brainfuck(Context* context);
int parse_brainfuck(Context* context, vword_t addr, char* prg);
int parse_brainfuck(Context* context, vword_t addr, char* prg, char* end);
vword_t ll_parse_brainfuck(Context* context, vword_t p);

#endif

