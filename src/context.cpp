#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <logger.h>
#include <context.h>

Logger* Context::logger = new Logger("context");

Context::Context(size_t page_size_, int num_pages_)
    : page_size(page_size_)
{
    this->num_pages = 0;
    this->pages = (struct page*) NULL;
    this->resize(num_pages_);
}

Context::~Context()
{
    int i;
    for(i = 0; i < this->num_pages; i++)
        free(this->pages[i].base);

    free((void*)this->pages);
}

void Context::dump(vword_t start, size_t length)
{
    printf("MEMORY-DUMP\n");

    vword_t* ptr = (vword_t*) calloc(length, sizeof(vbyte_t));
    this->read(start, length, (vbyte_t*) ptr);

    printf("...\n");

    int i;
    for(i = 0; i < length; i += sizeof(vword_t))
    {
        printf("0x%X: 0x%x\n", start+i, *ptr++);
    }

    printf("...\n");

    free(ptr);
}

void Context::resize(unsigned int num_pages_)
{
    if(num_pages_ < this->num_pages)
    {
        int i;
        for(i = num_pages_; i < this->num_pages; i++)
        {
            free(this->pages[i].base);
        }
    }

    this->pages = (struct page*) realloc(this->pages, num_pages_ * sizeof(struct page));

    if(num_pages_ > this->num_pages)
    {
        int i;
        for(i = this->num_pages; i < num_pages_; i++)
        {
            this->pages[i].base = NULL;
        }
    }

    this->num_pages = num_pages_;
}

int Context::read(vword_t addr, size_t length, vbyte_t* buf)
{
    int page_id = addr / this->page_size;
    int page_offset = addr % this->page_size;

    int end_page_id = (addr+length) / this->page_size;
    int end_page_offset = (addr+length) % this->page_size;

    if(end_page_id >= this->num_pages && end_page_offset > 0)
    {
        this->logger->log(lerror, "try to read out of memory (limit=0x%x)", this->num_pages * this->page_size);
        return 0;
    }

    while(page_id <= end_page_id)
    {
        if(this->pages[page_id].base == NULL)
            this->pages[page_id].base = calloc(this->page_size, sizeof(vbyte_t));

        int poe = (page_id == end_page_id) ? end_page_offset : this->page_size;

        while(page_offset < poe)
            *buf++ = ((vbyte_t*)this->pages[page_id].base)[page_offset++];

        page_offset = 0;
        page_id++;
    }

    return length;
}

int Context::write(vword_t addr, size_t length, vbyte_t* buf)
{
    int page_id = addr / this->page_size;
    int page_offset = addr % this->page_size;

    int end_page_id = (addr+length) / this->page_size;
    int end_page_offset = (addr+length) % this->page_size;

    if(end_page_id >= this->num_pages && end_page_offset > 0)
    {
        this->logger->log(lerror, "try to write out of memory (limit=0x%x)", this->num_pages * this->page_size);
        return 0;
    }

    while(page_id <= end_page_id)
    {
        if(this->pages[page_id].base == NULL)
            this->pages[page_id].base = calloc(this->page_size, sizeof(vbyte_t));

        int poe = (page_id == end_page_id) ? end_page_offset : this->page_size;
        while(page_offset < poe)
            ((vbyte_t*)this->pages[page_id].base)[page_offset++] = *buf++;

        page_offset = 0;
        page_id++;
    }

    return length;
}

vword_t Context::read_word(vword_t addr)
{
    vword_t val;
    this->read(addr, sizeof(vword_t), (vbyte_t*) &val);

    return val;
}

void Context::write_word(vword_t addr, vword_t val)
{
    this->write(addr, sizeof(vword_t), (vbyte_t*) &val);
}

vword_t Context::add_ll_fn(vword_t (*fn)(Context*, vword_t))
{
    static vword_t addr = 0x1;
    vword_t ret = addr;

    vword_t buf[32];
    vword_t* p = (vword_t*) buf;
    *p++ = -1;
    *((void**) p) = (void*) fn;
    this->write(addr, sizeof(vword_t) + sizeof(void*), (vbyte_t*) buf);

    addr += sizeof(vword_t) + sizeof(void*);

    return ret;
}

