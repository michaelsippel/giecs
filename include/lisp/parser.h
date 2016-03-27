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

int asm_parse(Context* context, vword_t addr, SNode* ast);
int asm_parse_list(Context* context, vword_t addr, SNode* ast);

size_t lisp_parse_size(SNode* ast);

int lisp_parse(Context* context, vword_t addr, SNode* ast);
int lisp_parse_list(Context* context, vword_t addr, SNode* ast);
int lisp_parse_symbol(Context* context, vword_t addr, SNode* ast);
int lisp_parse_string(Context* context, vword_t addr, SNode* ast);
int lisp_parse_integer(Context* context, vword_t addr, SNode* ast);

#endif

