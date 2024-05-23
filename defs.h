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
// symbol table length
#define SYM_TAB_LEN 1024
// when no register is available
#define NO_REG (-1)

// token type
enum {
    T_EOF,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_EQ,
    T_NE,
    T_LT,
    T_GT,
    T_LE,
    T_GE,
    T_INTLIT,
    T_SEMI,
    T_ASSIGN,
    T_IDENT,
    T_LBRACE,
    T_RBRACE,
    T_LPAREN,
    T_RPAREN,
    // keywords
    T_PRINT,
    T_INT,
    T_IF,
    T_ELSE,
    T_WHILE,
    T_FOR,
    T_VOID
};

// token struct
typedef struct token {
    int token;
    int intvalue;
} Token;

// AST node types
enum {
    A_ADD = 1,
    A_SUBTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_EQ,
    A_NE,
    A_LT,
    A_GT,
    A_LE,
    A_GE,
    A_INTLIT,
    A_IDENT,
    A_LVIDENT,
    A_ASSIGN,
    A_PRINT,
    A_GLUE,
    A_IF,
    A_WHILE,
    A_FUNCTION
};

// AST node struct
typedef struct ASTnode {
    int op;
    struct ASTnode *left;
    struct ASTnode *mid;
    struct ASTnode *right;
    union value {
        int intvalue; // intlit
        int id; // symbol solt id
    } value;
} ASTnode;

// symbol table struct
typedef struct Symbol {
    char *name;
} Symbol;

#endif //ACWJ_LEARN_DEFS_H
