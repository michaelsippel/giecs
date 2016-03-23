#include <string.h>

#include <logger.h>
#include <context.h>
#include <lisp/reader.h>
#include <lisp/parser.h>

extern Logger* lisp_parser_logger;
extern Logger* lisp_atom_logger;

int lisp_parse_symbol(Context* context, vword_t addr, SNode* ast)
{
    vword_t res = resolve_symbol(ast->string);
    if(res != 0)
    {
        context->write_word(addr, res);
    }
    else
    {
        lisp_atom_logger->log(lerror, "couldn't resolve symbol \"%s\"", ast->string);
    }

    return sizeof(vword_t);
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
    return sizeof(vword_t);
}

