#ifndef _context_h_
#define _context_h_

#include <stdint.h>

typedef uint8_t vbyte_t;
typedef uint32_t vword_t;

struct page
{
    unsigned int used_bytes;
    void* base;
};

// TODO
class Context
{
    public:
        Context(size_t page_size_);
        ~Context();

        void resize(unsigned int num_pages_);

        vbyte_t read(vword_t addr);
        void write(vword_t addr, vbyte_t value);

        void* base(vword_t addr);

    private:
        size_t page_size;

        unsigned int num_pages;
        struct page* pages;
};

#endif

