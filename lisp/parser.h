#ifndef _parser_h_
#define _parser_h_

#include <giecs/context.h>
#include <lisp/reader.h>

class Namespace;

struct symbol
{
    char* name;
    vword_t start;
    Namespace* ns;
};

struct parsepoint
{
    vword_t addr;
    size_t reqb;
};

class Namespace
{
    public:
        Namespace(Namespace* parent);
        ~Namespace();

        void add_parsepoint(vword_t addr, size_t reqb);
        void remove_parsepoint(vword_t addr);
        size_t get_reqb(vword_t addr);

        struct symbol* resolve_symbol(vword_t addr);
        struct symbol* resolve_symbol(const char* name);
        struct symbol* resolve_symbol(char* name);

        void add_symbol(const char* name, vword_t start);
        void add_symbol(const char* name, vword_t start, size_t reqb);
        void add_symbol(const char* name, vword_t start, size_t reqb, Namespace* ns);
        void add_symbol(char* name, vword_t start);
        void add_symbol(char* name, vword_t start, size_t reqb);
        void add_symbol(char* name, vword_t start, size_t reqb, Namespace* ns);

        void remove_symbol(vword_t addr);
        void remove_symbol(char* name);

    private:
        Namespace* parent;
        List<struct symbol*>* symbols;
        List<struct parsepoint>* parselist;
};

void add_parsepoint(vword_t addr, size_t reqb);
void remove_parsepoint(vword_t addr);
size_t get_reqb(vword_t addr);

struct symbol* resolve_symbol(vword_t addr);
struct symbol* resolve_symbol(const char* name);
struct symbol* resolve_symbol(char* name);

void add_symbol(const char* name, vword_t start);
void add_symbol(const char* name, vword_t start, size_t reqb);
void add_symbol(const char* name, vword_t start, size_t reqb, Namespace* ns);
void add_symbol(char* name, vword_t start);
void add_symbol(char* name, vword_t start, size_t reqb);
void add_symbol(char* name, vword_t start, size_t reqb, Namespace* ns);

void remove_symbol(vword_t addr);
void remove_symbol(char* name);


int asm_parse(Context* context, vword_t addr, SNode* ast);
int asm_parse_list(Context* context, vword_t addr, SNode* ast);

size_t lisp_parse_size(SNode* ast);

int lisp_parse(Context* context, vword_t addr, SNode* ast);
int lisp_parse_list(Context* context, vword_t addr, SNode* ast);
int asm_parse_symbol(Context* context, vword_t addr, SNode* ast);
int lisp_parse_symbol(Context* context, vword_t addr, SNode* ast);
int lisp_parse_string(Context* context, vword_t addr, SNode* ast);
int lisp_parse_integer(Context* context, vword_t addr, SNode* ast);

#endif

