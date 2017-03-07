
#pragma once

#include <istream>
#include <sstream>

namespace repl
{

class Language
{
    public:
        Language() {}
        virtual ~Language() {}

        virtual Language* parse(std::istream& stream) {}
        virtual void name(char* buf) {}

        Language* parse(std::string const& str)
        {
            std::istream* stream = new std::istringstream(str);
            Language* r = this->parse(*stream);
            delete stream;
            return r;
        }
};

} // namespace repl

