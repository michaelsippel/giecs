#include <string.h>

#include <context.h>
#include <reader.h>
#include <parser.h>

int parse_symbol(Context* context, vword_t addr, SNode* ast)
{
    *((vword_t*) context->base(addr)) = resolve_symbol(ast->string);
    return sizeof(vword_t);
}

int parse_string(Context* context, vword_t addr, SNode* ast)
{
    strcpy((char*) context->base(addr), ast->string);
    return strlen(ast->string);
}

int parse_integer(Context* context, vword_t addr, SNode* ast)
{
    *((vword_t*) context->base(addr)) = (vword_t) ast->integer;
    return sizeof(vword_t);
}
