//
// Created by tika on 24-5-19.
//
// Structure and enum definitions

#ifndef ACWJ_LEARN_DEFS_H
#define ACWJ_LEARN_DEFS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// symbol length
#define TEXT_LEN 512

// tokens
enum {
    T_EOF,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_INTLIT,
    T_SEMI,
    T_PRINT
};

// token struct
typedef struct token {
    int token;
    int intvalue;
} Token;

// AST node types
enum {
    A_ADD,
    A_SUBTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_INTLIT
};

// AST node struct
typedef struct ASTnode {
    int op;
    struct ASTnode *left;
    struct ASTnode *right;
    int intvalue;
} ASTnode;

#endif //ACWJ_LEARN_DEFS_H
