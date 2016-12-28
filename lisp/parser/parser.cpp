#include <giecs/context.h>
#include <lisp/reader.h>
#include <lisp/parser.h>

int asm_parse(Context* context, vword_t addr, SNode* ast)
{
    switch(ast->type)
    {
        case LIST:
            return asm_parse_list(context, addr, ast);
        case SYMBOL:
            return asm_parse_symbol(context, addr, ast);
        case STRING:
            return lisp_parse_string(context, addr, ast);
        case INTEGER:
            return lisp_parse_integer(context, addr, ast);
    }
    return 0;
}

int lisp_parse(Context* context, vword_t addr, SNode* ast)
{
    switch(ast->type)
    {
        case LIST:
            return lisp_parse_list(context, addr, ast);
        case SYMBOL:
            return lisp_parse_symbol(context, addr, ast);
        case STRING:
            return lisp_parse_string(context, addr, ast);
        case INTEGER:
            return lisp_parse_integer(context, addr, ast);
    }
    return 0;
}

