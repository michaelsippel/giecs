
#ifndef _stackframe_h_
#define _stackframe_h_

#include <logger.h>
#include <itype.h>

class Context;

class StackFrame
{
    public:
        StackFrame(Context* context_, vword_t base_);
        ~StackFrame();

        int move(int offset);
        int offset(void);
        vword_t ptr(void);

        int push(vbyte_t* buf, size_t len);
        int pop(vbyte_t* buf, size_t len);

        vbyte_t pop_byte(void);
        vword_t pop_word(void);
        void push_byte(vbyte_t v);
        void push_word(vword_t v);

        int peek(vbyte_t* buf, size_t len);
        vbyte_t peek_byte(void);
        vword_t peek_word(void);

        template <typename T>
        T* map(size_t len)
        {
            return (T*) this->map_void(len);
        }

        template <typename T>
        int unmap(T* buf)
        {
            return this->unmap_void((void*) buf);
        }

        Context* context;

    private:
        void* map_void(size_t len);
        int unmap_void(void* buf);

        vword_t base;
        int pos;

        int off_max;
        int off_min;

        static Logger* logger;
};

#endif

