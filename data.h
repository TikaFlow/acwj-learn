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
EXTERN_ int FLAG_LINE_START;
// input and output file
EXTERN_ FILE *IN_FILE;
EXTERN_ char *IN_FILE_NAME;
EXTERN_ FILE *OUT_FILE;
EXTERN_ char *OUT_FILE_NAME;
// last token
EXTERN_ Token TOKEN;
// putback token
EXTERN_ Token TOKEN_BACK;
// last symbol
EXTERN_ char TEXT[MAX_TEXT + 1];
// putback symbol
EXTERN_ char TEXT_BACK[MAX_TEXT + 1];
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
// current loop level
EXTERN_ int LOOP_LEVEL;
// current switch level
EXTERN_ int SWITCH_LEVEL;

// runtime flags
// verbose mode
EXTERN_ int FLAG_v;
// keep object file
EXTERN_ int FLAG_c;
// dump AST tree
EXTERN_ int FLAG_T;
// keep assembler file
EXTERN_ int FLAG_S;
// dump symbol table
EXTERN_ int FLAG_M;

#endif //ACWJ_LEARN_DATA_H
