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
// true and false
#define TRUE 1
#define FALSE 0

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
    T_VOID,
    T_CHAR
};

// token struct
typedef struct token {
    int token;
    int intvalue;
} Token;

// AST node types
enum {
    A_NONE,
    A_ADD,
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
    A_FUNCTION,
    A_WIDEN
};

// primitive type
enum {
    P_NONE,
    P_VOID,
    P_CHAR,
    P_INT
};

// struct type
enum {
    S_VARIABLE,
    S_FUNCTION
};

// AST node struct
typedef struct ASTnode {
    int op;
    int type;
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
    int ptype;
    int stype;
} Symbol;

#endif //ACWJ_LEARN_DEFS_H
