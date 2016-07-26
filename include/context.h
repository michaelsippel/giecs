#ifndef _context_h_
#define _context_h_

#include <stddef.h>
#include <stdint.h>

#include <logger.h>

typedef uint8_t vbyte_t;
typedef uint32_t vword_t;
#define VWORD_SIZE (sizeof(vword_t) / sizeof(vbyte_t))
#define IWORD_SIZE (sizeof(void*) / sizeof(vbyte_t))

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

		vword_t lower_limit(void);
		vword_t upper_limit(void);

        void resize(unsigned int num_pages_);

        void dump(vword_t start, size_t length);

        int read(vword_t addr, size_t length, vbyte_t* buf);
        int write(vword_t addr, size_t length, vbyte_t* buf);

        size_t read_str(vword_t addr, vbyte_t* buf);

        vword_t read_word(vword_t addr);
        void write_word(vword_t addr, vword_t val);

        void copy(vword_t dest, vword_t src, size_t length);

        vword_t add_ll_fn(vword_t (*fn)(Context*, vword_t));

    private:
        static Logger* logger;

        size_t page_size;

        unsigned int num_pages;
        struct page* pages;
};

#endif

