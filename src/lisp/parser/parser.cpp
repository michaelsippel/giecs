#include <context.h>
#include <reader.h>
#include <parser.h>

int parse(Context* context, vword_t addr, SNode* ast)
{
    switch(ast->type)
    {
        case LIST:
            return parse_list(context, addr, ast);
        case SYMBOL:
            return parse_symbol(context, addr, ast);
        case STRING:
            return parse_string(context, addr, ast);
        case INTEGER:
            return parse_integer(context, addr, ast);
    }
    return 0;
}

int parse_se(Context* context, vword_t addr, SNode* ast)
{
    switch(ast->type)
    {
        case LIST:
            return parse_list_se(context, addr, ast);
        case SYMBOL:
            return parse_symbol(context, addr, ast);
        case STRING:
            return parse_string(context, addr, ast);
        case INTEGER:
            return parse_integer(context, addr, ast);
    }
    return 0;
}

