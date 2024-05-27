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
// when no label is available
#define NO_LABEL 0
// true and false
#define TRUE 1
#define FALSE 0

// token type
enum {
    T_EOF,
    // binary operators
    T_ASSIGN,
    T_LOGOR,
    T_LOGAND,
    T_OR,
    T_XOR,
    T_AND,
    T_EQ,
    T_NE,
    T_LT,
    T_GT,
    T_LE,
    T_GE,
    T_LSHIFT,
    T_RSHIFT,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    // unary operators
    T_INC,
    T_DEC,
    T_INVERT,
    T_LOGNOT,
    // types
    T_VOID,
    T_CHAR,
    T_INT,
    T_LONG,
    // keywords
    T_IF,
    T_ELSE,
    T_WHILE,
    T_FOR,
    T_RETURN,
    // structures
    T_INTLIT,
    T_STRLIT,
    T_SEMI,
    T_IDENT,
    T_LBRACE,
    T_RBRACE,
    T_LPAREN,
    T_RPAREN,
    T_LBRACKET,
    T_RBRACKET,
    T_COMMA,
};

// token struct
typedef struct Token {
    int token_type;
    long int_value;
} Token;

// AST node type
enum {
    A_NONE,
    A_ASSIGN,
    A_LOGOR,
    A_LOGAND,
    A_OR,
    A_XOR,
    A_AND,
    A_EQ,
    A_NE,
    A_LT,
    A_GT,
    A_LE,
    A_GE,
    A_LSHIFT,
    A_RSHIFT,
    A_ADD,
    A_SUBTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_INTLIT,
    A_STRLIT,
    A_IDENT,
    A_GLUE,
    A_IF,
    A_WHILE,
    A_FUNCTION,
    A_WIDEN,
    A_RETURN,
    A_FUNCCALL,
    A_DEREF,
    A_ADDR,
    A_SCALE,
    A_PREINC,
    A_PREDEC,
    A_POSTINC,
    A_POSTDEC,
    A_NEGATE,
    A_INVERT,
    A_LOGNOT,
    A_TOBOOL
};

// primitive type
enum {
    P_NONE,
    P_VOID,
    P_CHAR,
    P_INT,
    P_LONG,
    P_VOIDPTR,
    P_CHARPTR,
    P_INTPTR,
    P_LONGPTR
};

// struct type
enum {
    S_VARIABLE,
    S_FUNCTION,
    S_ARRAY
};

// AST node struct
typedef struct ASTnode {
    int op;
    int type;
    int rvalue;
    struct ASTnode *left;
    struct ASTnode *mid;
    struct ASTnode *right;
    union value {
        long int_value; // intlit
        int id; // symbol slot id
        int size; // scale
    } value;
} ASTnode;

// symbol table struct
typedef struct Symbol {
    char *name;
    int ptype;
    int stype;
    int end_label;
    int size;
} Symbol;

#endif //ACWJ_LEARN_DEFS_H
