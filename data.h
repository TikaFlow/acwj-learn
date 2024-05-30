//
// Created by tika on 24-5-19.
//
// Global variables

#ifndef ACWJ_LEARN_DATA_H
#define ACWJ_LEARN_DATA_H

#include "defs.h"

#ifndef extern_
#define extern_ extern
#endif

// current line
extern_ int LINE;
// put back char
extern_ int PUT_BACK;
// input and output file
extern_ FILE *IN_FILE;
extern_ FILE *OUT_FILE;
extern_ char *OUT_FILE_NAME;
// last token
extern_ Token TOKEN;
// reject token
extern_ Token TOKEN_BACK;
// last symbol
extern_ char TEXT[MAX_TEXT + 1];
// symbol table
extern_ Symbol SYM_TAB[MAX_SYM];
// next free GLOBAL symbol slot
extern_ int GLOBAL_TOP;
// next free LOCAL symbol slot
extern_ int LOCAL_TOP;
// current function id
extern_ int FUNC_ID;

// runtime flags
// verbose mode
extern_ int FLAG_v;
// keep object file
extern_ int FLAG_c;
// keep assembler file
extern_ int FLAG_S;
// dump AST tree
extern_ int FLAG_T;

#endif //ACWJ_LEARN_DATA_H
