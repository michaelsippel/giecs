#include <string.h>

#include <giecs/logger.h>
#include <giecs/context.h>
#include <giecs/lisp/reader.h>
#include <giecs/lisp/parser.h>

extern Logger* lisp_parser_logger;
extern Logger* lisp_atom_logger;

int asm_parse_symbol(Context* context, vword_t addr, SNode* ast)
{
    struct symbol* sym = resolve_symbol(ast->string);
    if(sym != NULL)
    {
        context->write_word(addr, sym->start);
    }
    else
    {
        lisp_atom_logger->log(lerror, "couldn't resolve symbol \"%s\"", ast->string);
    }

    return VWORD_SIZE;
}

int lisp_parse_symbol(Context* context, vword_t addr, SNode* ast)
{
    struct symbol* sym = resolve_symbol(ast->string);
    if(sym != NULL)
    {
        context->write_word(addr, sym->start);
        return VWORD_SIZE;
    }
    else
    {
        lisp_atom_logger->log(lerror, "couldn't resolve symbol \"%s\"", ast->string);
        return -1;
    }

    return 0;
}

int lisp_parse_string(Context* context, vword_t addr, SNode* ast)
{
    int len = strlen(ast->string)+1;
    context->write(addr, len, (vbyte_t*) ast->string);
    return len;
}

int lisp_parse_integer(Context* context, vword_t addr, SNode* ast)
{
    context->write_word(addr, (vword_t) ast->integer);
    return VWORD_SIZE;
}

