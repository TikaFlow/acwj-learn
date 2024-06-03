//
// Created by tika on 24-5-19.
//
// Global variables

#ifndef ACWJ_LEARN_DATA_H
#define ACWJ_LEARN_DATA_H

#include "defs.h"

#ifndef EXTERN_
#define EXTERN_ extern
#endif

// current line
EXTERN_ int LINE;
// put back char
EXTERN_ int PUT_BACK;
// input and output file
EXTERN_ FILE *IN_FILE;
EXTERN_ char *IN_FILE_NAME;
EXTERN_ FILE *OUT_FILE;
EXTERN_ char *OUT_FILE_NAME;
// last token
EXTERN_ Token TOKEN;
// reject token
EXTERN_ Token TOKEN_BACK;
// last symbol
EXTERN_ char TEXT[MAX_TEXT + 1];
// symbol table of global/local/param/composite
EXTERN_ Symbol *GLOBAL_HEAD, *GLOBAL_TAIL;
EXTERN_ Symbol *LOCAL_HEAD, *LOCAL_TAIL;
EXTERN_ Symbol *PARAM_HEAD, *PARAM_TAIL;
EXTERN_ Symbol *STRUCT_HEAD, *STRUCT_TAIL;
EXTERN_ Symbol *UNION_HEAD, *UNION_TAIL;
EXTERN_ Symbol *MEMBER_HEAD, *MEMBER_TAIL;
EXTERN_ Symbol *ENUM_HEAD, *ENUM_TAIL;
EXTERN_ Symbol *TYPEDEF_HEAD, *TYPEDEF_TAIL;
// current function pointer
EXTERN_ Symbol *FUNC_PTR;

// runtime flags
// verbose mode
EXTERN_ int FLAG_v;
// keep object file
EXTERN_ int FLAG_c;
// keep assembler file
EXTERN_ int FLAG_S;
// dump AST tree
EXTERN_ int FLAG_T;

#endif //ACWJ_LEARN_DATA_H
