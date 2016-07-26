
#include <context.h>
#include <stackframe.h>
#include <logger.h>

Logger* StackFrame::logger = new Logger("StackFrame");

StackFrame::StackFrame(Context* context_, vword_t base_)
    : context(context_), base(base_)
{
    this->pos = 0;

    this->off_max = this->context->upper_limit() - base;
    this->off_min = this->context->lower_limit() - base;
}

StackFrame::~StackFrame()
{
}

int StackFrame::move(int offset)
{
    static Logger* logger = new Logger(this->logger, "move");

    if(offset > this->off_max)
    {
        logger->log(lerror, "reached upper limit, 0x%x + %x > 0x%x", this->ptr(), offset, this->context->upper_limit());
        offset = this->off_max;
    }

    if(offset < this->off_min)
    {
        logger->log(lerror, "reached lower limit, 0x%x - %x < 0x%x", this->ptr(), -offset, this->context->lower_limit());
        offset = this->off_min;
    }

    this->off_max -= offset;
    this->off_min += offset;

    this->pos += offset;
    return offset;
}

int StackFrame::offset(void)
{
    return this->pos;
}

vword_t StackFrame::ptr(void)
{
    return this->base + this->pos;
}

int StackFrame::push(vbyte_t* buf, size_t len)
{
    int l = this->move(-len);
    vword_t p = this->ptr();
    this->context->write(p, -l, buf);
    return l;
}

int StackFrame::pop(vbyte_t* buf, size_t len)
{
    vword_t p = this->ptr();
    len = this->move(len);
    len = this->context->read(p, len, buf);
    return len;
}

vbyte_t StackFrame::pop_byte(void)
{
    vbyte_t v;
    this->pop((vbyte_t*) &v, 1);
    return v;
}

vword_t StackFrame::pop_word(void)
{
    vword_t v;
    this->pop((vbyte_t*) &v, VWORD_SIZE);
    return v;
}

void StackFrame::push_byte(vbyte_t v)
{
    this->push((vbyte_t*) &v, 1);
}

void StackFrame::push_word(vword_t v)
{
    this->push((vbyte_t*) &v, VWORD_SIZE);
}

void* StackFrame::map_void(size_t len)
{
    //TODO
    return NULL;
}

int StackFrame::unmap_void(void* buf)
{
    // TODO
    return 0;
}

