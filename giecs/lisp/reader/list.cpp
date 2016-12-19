#include <string.h>

#include <giecs/logger.h>
#include <giecs/lisp/reader.h>

void SNode::read_list(char* start, char* end)
{
    static Logger* logger = new Logger("reader");

    this->type = LIST;
    this->subnodes = new List<SNode*>();

    enum snode_type st = (enum snode_type) -1;
    char* s;
    char* e;
    bool string = false;
    bool comment = false;
    bool quote = false;
    int depth = 0;

    char* str = start;
    while(str < end)
    {
        if(st == (enum snode_type)-1 && !comment)
        {
            switch(*str)
            {
                case ')':
                    logger->log(lerror, "unexpected )");
                    return;

                case '(':
                    st = LIST;
                    str++;
                    depth++;
                    break;

                case '"':
                    st = STRING;
                    string = true;
                    str++;
                    break;

                case '\'':
                    quote = true;
                    break;

                case ';':
                case '#':
                    comment = true;

                case '\n':
                case ' ':
                case '\t':
                    break;

                default:
                    if( (*str >= '0' && *str <= '9') ||
                            (*str == '-' && *(str+1) >= '0' && *(str+1) <= '9'))
                        st = INTEGER;
                    else
                        st = SYMBOL;
                    break;
            }
            s = str;
        }

        if(st != (enum snode_type) -1 && !comment)
        {
            switch(*str)
            {
                case '(':
                    depth++;
                    break;

                case ')':
                    depth--;
                    if(!string)
                        goto add;

                    break;

                case '"':
                    if(string)
                        goto add;
                    else if(depth == 0)
                    {
                        logger->log(lerror, "unexpexted \"");
                        return;
                    }

                    break;

                case ';':
                case '#':
                    if(!string)
                        comment = true;
                    else
                        break;

                case '\n':
                case '\t':
                case ' ':
                    if(!string)
                        goto add;

                default:
                    break;
add:
                    if(depth == 0)
                    {
                        SNode* sn;
                        if(quote)
                        {
                            sn = new SNode(LIST);
                            SNode* sq = new SNode(st);
                            sq->read(s, str);

                            sn->subnodes = new List<SNode*>();
                            sn->subnodes->pushBack(new SNode(SYMBOL, (char*)"quote"));
                            sn->subnodes->pushBack(sq);
                        }
                        else
                        {
                            sn = new SNode(st);
                            sn->read(s, str);
                        }

                        this->subnodes->pushBack(sn);
                        string = false;
                        quote = false;
                        st = (enum snode_type)-1;
                    }
            }
        }
        else
        {
            if(*str == '\n')
                comment = false;
        }
        str++;
    }

    depth = 0;
    if(st != (enum snode_type)-1)
        goto add;
}

