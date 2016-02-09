#ifndef _context_h_
#define _context_h_

#include <stddef.h>
#include <stdint.h>

#include <logger.h>

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
        Context(size_t page_size_, int num_pages_);
        ~Context();

        void resize(unsigned int num_pages_);

        void dump(vword_t start, size_t length);

        int read(vword_t addr, size_t length, vbyte_t* buf);
        int write(vword_t addr, size_t length, vbyte_t* buf);

        void* base(vword_t addr);

        vword_t add_ll_fn(vword_t (*fn)(Context*, vword_t));

    private:
        static Logger* logger;

        size_t page_size;

        unsigned int num_pages;
        struct page* pages;
};

#endif

