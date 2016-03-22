#include <string.h>

#include <logger.h>
#include <context.h>
#include <lisp/reader.h>
#include <lisp/parser.h>

static Logger* parser_logger = new Logger("parser");
static Logger* atom_logger = new Logger(parser_logger, "atom");

int parse_symbol(Context* context, vword_t addr, SNode* ast)
{
    vword_t res = resolve_symbol(ast->string);
    if(res != 0)
    {
        context->write_word(addr, res);
    }
    else
    {
        atom_logger->log(lerror, "couldn't resolve symbol \"%s\"", ast->string);
    }

    return sizeof(vword_t);
}

int parse_string(Context* context, vword_t addr, SNode* ast)
{
    int len = strlen(ast->string)+1;
    context->write(addr, len, (vbyte_t*) ast->string);
    return len;
}

int parse_integer(Context* context, vword_t addr, SNode* ast)
{
    context->write_word(addr, (vword_t) ast->integer);
    return sizeof(vword_t);
}

