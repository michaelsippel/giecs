#ifndef _lisp_ll_h_
#define _lisp_ll_h_

#include <giecs/context.hpp>
#include <giecs/lisp/reader.hpp>

void init_lisp(Context* context);

void ll_quote(StackFrame& stack);
void ll_asm(StackFrame& stack);

void ll_expand(StackFrame& stack);
size_t expand(Context* context, vword_t pt, vword_t* p, bool quoted, bool quoteptr);

size_t expand_evalparam(Context* context, vword_t p, vword_t fn, List<SNode*>* plist);
vword_t expand_macro(Context* context, vword_t pt, SNode* plist, SNode* val, vword_t* p);

void ll_eval_param(StackFrame& stack);
void ll_expand_macro(StackFrame& stack);

void ll_function(StackFrame& stack);
void ll_macro(StackFrame& stack);

void ll_declare(StackFrame& stack);
void ll_isdef(StackFrame& stack);

void ll_lmap(StackFrame& stack);
void ll_progn(StackFrame& stack);

#endif

