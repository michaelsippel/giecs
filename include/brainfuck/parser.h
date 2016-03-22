#ifndef _brainfuck_parser_h_
#define _brainfuck_parser_h_

void init_brainfuck(Context* context);
int parse_brainfuck(Context* context, vword_t addr, char* prg);
int parse_brainfuck(Context* context, vword_t addr, char* prg, char* end);
vword_t ll_parse_brainfuck(Context* context, vword_t p);

#endif

