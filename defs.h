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
#include <errno.h>
#include <unistd.h>

// symbol length
#define MAX_TEXT 1024
// max number of obj files
#define MAX_OBJ 256
// when no register is available
#define NO_REG (-1)
// when no label is available
#define NO_LABEL 0
// true and false
#define TRUE 1
#define FALSE 0
// align direction
#define ASC 1
#define DESC (-1)
// when not found, return this value - function that returns index
#define NOT_FOUND (-1)
// default output file name
#define A_OUT "a.out"
// pre-processor command
#define CPP_CMD "cpp -nostdinc -isystem"
// need define include dir when compile
#ifndef INC_DIR
#warning "INC_DIR is not defined, using default value: /usr/include"
#define INC_DIR "/usr/include"
#endif
// assembler command
#define AS_CMD "as -o"
// linker command
#define LD_CMD "ld -o"
#define LD_SUFFIX "/lib/x86_64-linux-gnu/crt1.o -lc -I /lib64/ld-linux-x86-64.so.2"

// typedefs
typedef struct Symbol Symbol;
typedef struct ASTnode ASTnode;
typedef struct Token Token;

// token type
enum {
    T_EOF,
    // binary operators
    T_ASSIGN, T_LOGOR, T_LOGAND, T_OR, T_XOR, T_AND,
    T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE,
    T_LSHIFT, T_RSHIFT, T_PLUS, T_MINUS, T_STAR, T_SLASH,
    // unary operators
    T_INC, T_DEC, T_INVERT, T_LOGNOT,
    // types
    T_VOID, T_CHAR, T_INT, T_LONG,
    // keywords
    T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
    T_STRUCT, T_UNION, T_ENUM, T_TYPEDEF, T_EXTERN,
    T_BREAK, T_CONTINUE, T_SWITCH, T_CASE, T_DEFAULT,
    // structures
    T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_LBRACKET, T_RBRACKET,
    T_COMMA, T_DOT, T_ARROW, T_COLON,
};

// AST node type
enum {
    A_NONE,
    A_ASSIGN, A_LOGOR, A_LOGAND, A_OR, A_XOR, A_AND,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
    A_LSHIFT, A_RSHIFT,
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
    A_INTLIT, A_STRLIT, A_IDENT, A_GLUE,
    A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN, A_FUNCCALL,
    A_DEREF, A_ADDR, A_SCALE,
    A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC, A_NEGATE, A_INVERT,
    A_LOGNOT, A_TOBOOL,
    A_BREAK, A_CONTINUE, A_SWITCH, A_CASE, A_DEFAULT,
};

// primitive type
enum {
    P_NONE = 0x00,
    P_VOID = 0x10,
    P_CHAR = 0x20,
    P_INT = 0x30,
    P_LONG = 0x40,
    P_STRUCT = 0x50,
    P_UNION = 0x60
};

// struct type
enum {
    S_NONE,
    S_VARIABLE,
    S_FUNCTION,
    S_ARRAY
};

// storage class: global or local
enum {
    C_NONE,
    C_GLOBAL,
    C_LOCAL,
    C_PARAM,
    C_EXTERN,
    C_STRUCT,
    C_UNION,
    C_MEMBER,
    C_ENUMTYPE,
    C_ENUMVAL,
    C_TYPEDEF
};

// token struct
struct Token {
    int token_type;
    long int_value;
};

// AST node struct
struct ASTnode {
    int op;
    int type;
    int rvalue;
    ASTnode *left;
    ASTnode *mid;
    ASTnode *right;
    Symbol *sym; // symbol pointer in the symbol table
    union {
        long int_value; // intlit
        int id; // symbol slot id
        int size; // scale
    };
};

// symbol table struct
struct Symbol {
    char *name;
    int ptype;
    int stype;
    Symbol *ctype; // for struct/union, pointer to it's type
    int class; // global/local/param/struct/union/member
    union {
        int end_label;
        int size;
    }; // element number of symbol/array
    union {
        int n_param; // for function, number of parameters
        int posn; // for param, positive position from stack base pointer/RBP
    };
    Symbol *next;
    Symbol *first;
};

#endif //ACWJ_LEARN_DEFS_H
