
#pragma once

#include <streambuf>

class Substreambuf : public std::streambuf
{
    public:
        Substreambuf(std::streambuf* source_, int start, int len)
            : source(source_), start(start), len(len), pos(0)
        {
            this->source->pubseekpos(start);
            this->setbuf(NULL, 0);
        }

        // ugly
        ~Substreambuf()
        {
            this->source->pubseekpos(start+len);
        }

    protected:
        int underflow(void)
        {
            if(this->pos+1 >= this->len)
                return traits_type::eof();

            return this->source->sgetc();
        }

        int uflow(void)
        {
            if(this->pos+1 >= this->len)
                return traits_type::eof();

            this->pos += 1;

            return this->source->sbumpc();
        }

        std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out )
        {
            switch(way)
            {
                case std::ios_base::beg:
                    this->pos = this->start + off;
                    break;
                case std::ios_base::cur:
                    this->pos += off;
                    break;
                case std::ios_base::end:
                    this->pos = this->len + off;
                    break;
            }

            if(this->pos < 0 || this->pos >= this->len)
                return -1;

            if(this->source->pubseekpos(this->start + this->pos, which) == -1)
                return -1;

            return this->pos;
        }

        std::streampos seekpos(std::streampos sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
        {
            this->pos = sp;

            if(this->pos < 0 || this->pos >= this->len)
                return -1;

            if(this->source->pubseekpos(this->start + this->pos, which) == -1)
                return -1;

            return this->pos;
        }

    private:
        std::streambuf* source;
        std::streampos start;
        std::streamsize len;
        std::streampos pos;
};

