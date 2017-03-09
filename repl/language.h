
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

        virtual int parse(std::istream& stream) {}
        virtual void name(char* buf) {}

        int parse(std::string const& str)
        {
            std::istream* stream = new std::istringstream(str);
            int r = this->parse(*stream);
            delete stream;
            return r;
        }
};

} // namespace repl

