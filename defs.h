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
#include <stdarg.h>

// constants
#define MAX_INT 0x7fffffff
#define DEFAULT_ARRAY_SIZE 0X10
// symbol length
#define MAX_TEXT 1024
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

// typedefs
typedef struct Symbol Symbol;
typedef struct ASTnode ASTnode;
typedef struct Token Token;

// value type, used to get name
enum {
    V_TOKEN,
    V_OP,
    V_PTYPE,
};

// token type
enum {
    T_EOF,
    // binary operators
    T_ASSIGN, T_ASPLUS, T_ASMINUS, T_ASSTAR, T_ASSLASH, T_ASMOD, // 6
    T_QUESTION, T_LOGOR, T_LOGAND, T_OR, T_XOR, T_AND, // 12
    T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE, // 18
    T_LSHIFT, T_RSHIFT, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_MOD, // 25
    // unary operators
    T_INC, T_DEC, T_INVERT, T_LOGNOT, // 29
    // types
    T_VOID, T_CHAR, T_SHORT, T_INT, T_LONG, // 34
    // keywords
    T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN, T_SIZEOF, // 40
    T_STRUCT, T_UNION, T_ENUM, T_TYPEDEF, T_EXTERN, // 45
    T_BREAK, T_CONTINUE, T_SWITCH, T_CASE, T_DEFAULT, // 50
    T_STATIC,
    // structures
    T_INTLIT, T_STRLIT, T_SEMI, T_IDENT, // 55
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_LBRACKET, T_RBRACKET, // 61
    T_COMMA, T_DOT, T_ARROW, T_COLON, // 65
};

// AST node type
enum {
    A_NONE,
    A_ASSIGN, A_ASPLUS, A_ASMINUS, A_ASSTAR, A_ASSLASH, A_ASMOD, // 6
    A_TERNARY, A_LOGOR, A_LOGAND, A_OR, A_XOR, A_AND, // 12
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, // 18
    A_LSHIFT, A_RSHIFT, A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_MOD, // 25
    A_INTLIT, A_STRLIT, A_IDENT, A_GLUE, // 29
    A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN, A_FUNCCALL, // 35
    A_DEREF, A_ADDR, A_SCALE, // 38
    A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC, A_NEGATE, A_INVERT, // 44
    A_LOGNOT, A_TOBOOL,
    A_BREAK, A_CONTINUE, A_SWITCH, A_CASE, A_DEFAULT, A_CAST, // 52
    A_NOP, A_DECLARE,
};

// primitive type
enum {
    P_NONE = 0x00,
    P_VOID = 0x10,
    P_CHAR = 0x20,
    P_SHORT = 0x30,
    P_INT = 0x40,
    P_LONG = 0x50,
    P_STRUCT = 0x60,
    P_UNION = 0x70
};

// struct type
enum {
    S_NONE,
    S_VARIABLE,
    S_FUNCTION,
    S_ARRAY,
    S_TYPE,
};

// storage class: global or local
enum {
    C_NONE,
    C_GLOBAL,
    C_LOCAL,
    C_PARAM,
    C_EXTERN,
    C_STATIC,
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

// symbol table struct
struct Symbol {
    char *name;
    int ptype;
    int stype;
    Symbol *ctype; // for struct/union, pointer to it's type
    int class; // global/local/param/struct/union/member
    int size; // total byte size
    int n_elem; // for function, number of parameters, for array, number of elements
    long *init_list; // init value list
    union {
        int end_label; // function end label
        int posn; // for param, positive position from stack base pointer/RBP
    };
    Symbol *next; // next symbol
    Symbol *first; // for struct/union, pointer to first member, for function, pointer to first param/local
};

// AST node struct
struct ASTnode {
    int op;
    int type;
    Symbol *ctype; // for struct/union, pointer to it's type
    int rvalue;
    ASTnode *left;
    ASTnode *mid;
    ASTnode *right;
    Symbol *sym; // symbol pointer in the symbol table
    union {
        long int_value; // intlit
        int size; // scale
    };
    int line; // line number which the node comes from
};

#endif //ACWJ_LEARN_DEFS_H
