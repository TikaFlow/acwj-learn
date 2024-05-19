//
// Created by tika on 24-5-19.
//

#ifndef ACWJ_LEARN_DEFS_H
#define ACWJ_LEARN_DEFS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// token struct
typedef struct token {
    int token;
    int intvalue;
} Token;

// tokens
enum {
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_INTLIT
};

#endif //ACWJ_LEARN_DEFS_H
