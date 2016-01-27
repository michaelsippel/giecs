#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <context.h>

Context::Context(size_t page_size_)
    : page_size(page_size_)
{
    this->num_pages = 1;
    this->pages = (struct page*) malloc(this->num_pages);
    this->pages[0].base = malloc(this->page_size);
}

Context::~Context()
{
    int i;
    for(i = 0; i < this->num_pages; i++)
        free(this->pages[i].base);

    free((void*)this->pages);
}

void Context::resize(unsigned int num_pages_)
{
    // TODO
    if(num_pages_ < this->num_pages)
    {
        int i;
        for(i = num_pages_; i < this->num_pages; i++)
        {
            free(this->pages[i].base);
        }
    }
    this->num_pages = num_pages_;
    this->pages = (struct page*) realloc(this->pages, this->num_pages);
}


void* Context::base(vword_t addr)
{
    // TODO
    return (void*) ((uintptr_t)this->pages[0].base + addr);
}

vword_t Context::add_ll_fn(vword_t (*fn)(Context*, vword_t))
{
    static vword_t addr = 0x1;
    vword_t ret = addr;

    vword_t* ptr = (vword_t*) this->base(addr);
    *ptr++ = -1;
    *((void**)ptr) = (void*) fn;
    addr += sizeof(vword_t) + sizeof(void*);

    return ret;
}

