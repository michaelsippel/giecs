#ifndef _reader_h_
#define _reader_h_

#include <list.h>
#include <context.h>

enum snode_type
{
    LIST,
    SYMBOL,
    STRING,
    INTEGER
};

class SNode
{
    public:
        SNode(enum snode_type type_);
        SNode(enum snode_type type_, char* src);
        SNode(Context* context, vword_t addr);
        ~SNode();

        enum snode_type type;
        union
        {
            List<SNode*>* subnodes;
            char* string;
            int integer;
        };

        void read(char* src);
        void read(char* start, char* end);
        void read_list(char* start, char* end);
        void read_symbol(char* start, char* end);
        void read_string(char* start, char* end);
        void read_integer(char* start, char* end);

        void dump(void);
        void dump(int indent);

        size_t vmem_size(void);
        size_t write_vmem(Context* context, vword_t addr);
        void read_vmem(Context* context, vword_t addr);
};

#endif

