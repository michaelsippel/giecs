#ifndef _parser_h_
#define _parser_h_

#include <context.h>
#include <lisp/reader.h>

class Namespace;

struct symbol
{
    char* name;
    vword_t start;

    size_t reqb;
    Namespace* ns;
};

class Namespace
{
    public:
        Namespace(Namespace* parent);
        ~Namespace();

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
};


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

