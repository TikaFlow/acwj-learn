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
#define SYM_TAB_LEN 1024

// token type
enum {
    T_EOF,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_INTLIT,
    T_SEMI,
    T_EQUALS,
    T_IDENT,
    T_PRINT,
    T_INT
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
    A_INTLIT,
    A_IDENT,
    A_LVIDENT,
    A_ASSIGN
};

// AST node struct
typedef struct ASTnode {
    int op;
    struct ASTnode *left;
    struct ASTnode *right;
    union value {
        int intvalue; //intlit
        int id; // symbol solt id
    } value;
} ASTnode;

// symbol table struct
typedef struct Symbol {
    char *name;
} Symbol;

#endif //ACWJ_LEARN_DEFS_H
