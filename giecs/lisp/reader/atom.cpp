#include <stdlib.h>
#include <string.h>

#include <lisp/reader.h>

void SNode::read_symbol(char* start, char* end)
{
    this->type = SYMBOL;

    size_t len = end - start;
    this->string = (char*) malloc(len + 1);
    memcpy(this->string, start, len);
    this->string[len] = '\0';
}

void SNode::read_string(char* start, char* end)
{
    this->type = STRING;

    size_t len = end - start;
    this->string = (char*) malloc(len + 1);

    char* dest = this->string;
    char* str = start;
    while(str < end)
    {
        if(*str == '\\')
        {
            switch(*++str)
            {
                case 'n':
                    *dest = '\n';
                    break;

                case 't':
                    *dest = '\t';
                    break;

                case 'v':
                    *dest = '\v';
                    break;

                case 'b':
                    *dest = '\b';
                    break;

                case 'r':
                    *dest = '\r';
                    break;

                case 'f':
                    *dest = '\f';
                    break;

                case 'a':
                    *dest = '\a';
                    break;

                case '\\':
                    *dest = '\\';
                    break;

                case '?':
                    *dest = '\?';
                    break;

                case '\'':
                    *dest = '\'';
                    break;

                case '\"':
                    *dest = '\"';
                    break;
            }
        }
        else
            *dest = *str;

        dest++;
        str++;
    }
    *dest = '\0';

    size_t nl = strlen(this->string) + 1;
    this->string = (char*) realloc((void*)this->string, nl);
}

void SNode::read_integer(char* start, char* end)
{
    this->type = INTEGER;

    size_t len = end - start;
    char* buf = (char*) malloc(len + 1);
    memcpy((void*)buf, (void*)start, len);
    buf[len] = '\0';

    this->integer = atoi(buf);

    free(buf);
}

