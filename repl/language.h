
#pragma once

namespace repl
{

class Language
{
    public:
        Language() {}
        virtual ~Language() {}
        virtual Language* parse(char* str) {}
        virtual void name(char* buf) {}
};

} // namespace repl

