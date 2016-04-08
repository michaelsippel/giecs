#include <string.h>

#include <logger.h>
#include <context.h>
#include <lisp/reader.h>
#include <lisp/parser.h>

extern Logger* lisp_parser_logger;
extern Logger* lisp_atom_logger;

int asm_parse_symbol(Context* context, vword_t addr, SNode* ast)
{
    vword_t res = resolve_symbol(ast->string).start;
    if(res != 0)
    {
        context->write_word(addr, res);
    }
    else
    {
        lisp_atom_logger->log(lerror, "couldn't resolve symbol \"%s\"", ast->string);
    }

    return VWORD_SIZE;
}

int lisp_parse_symbol(Context* context, vword_t addr, SNode* ast)
{
    struct symbol sym = resolve_symbol(ast->string);
    if(sym.start != 0)
    {
        if(sym.reqb > 0)
        {
            vword_t v[3];
            v[0] = resolve_symbol("genfn").start;
            v[1] = sym.start;
            v[2] = sym.reqb;

            context->write(addr, 3*VWORD_SIZE, (vbyte_t*) &v);
            return 3*VWORD_SIZE;
        }
        else
        {
            context->write_word(addr, sym.start);
            return VWORD_SIZE;
        }
    }
    else
    {
        lisp_atom_logger->log(lerror, "couldn't resolve symbol \"%s\"", ast->string);
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

