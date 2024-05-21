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
// last token
extern_ Token TOKEN;
// last symbol
extern_ char TEXT[TEXT_LEN + 1];
// symbol table
extern_ Symbol SYM_TAB[SYM_TAB_LEN];

#endif //ACWJ_LEARN_DATA_H
