#include <stdint.h>
#include <stdlib.h>

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

