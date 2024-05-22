//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static ASTnode *singlestmt();

static ASTnode *stmtprint() {
    match(T_PRINT, "print");
    ASTnode *tree = binexpr(0);
    tree = mkastunary(A_PRINT, tree, 0);

    return tree;
}

static ASTnode *stmtassign() {
    ASTnode *left, *right, *tree;
    int id;

    match(T_IDENT, "identifier");
    if ((id = findglob(TEXT)) < 0) {
        fatals("undeclared variable:", TEXT);
    }
    right = mkastleaf(A_LVIDENT, id);

    match(T_ASSIGN, "=");
    left = binexpr(0);

    tree = mkastnode(A_ASSIGN, left, NULL, right, 0);
    genfreeregs();

    return tree;
}

static ASTnode *stmtif() {
    ASTnode *condnode, *truenode, *falsenode = NULL;

    match(T_IF, "if");
    match(T_LPAREN, "(");
    condnode = binexpr(0);
    if (condnode->op < A_EQ || condnode->op > A_GE) {
        fatal("Bad comparison operator");
    }
    match(T_RPAREN, ")");
    truenode = compoundstmt();
    if (TOKEN.token == T_ELSE) {
        match(T_ELSE, "else");
        falsenode = compoundstmt();
    }

    return mkastnode(A_IF, condnode, truenode, falsenode, 0);
}

static ASTnode *stmtwhile() {
    ASTnode *condnode, *bodenode;

    match(T_WHILE, "while");
    match(T_LPAREN, "(");

    condnode = binexpr(0);
    if (condnode->op < A_EQ || condnode->op > A_GE) {
        fatal("Bad comparison operator");
    }
    match(T_RPAREN, ")");

    bodenode = compoundstmt();

    return mkastnode(A_WHILE, condnode, NULL, bodenode, 0);
}

static ASTnode *stmtfor() {
    ASTnode *condnode, *bodenode, *preopnode, *postopnode, *tree;

    match(T_FOR, "for");
    match(T_LPAREN, "(");

    preopnode = singlestmt();
    match(T_SEMI, ";");

    condnode = binexpr(0);
    if (condnode->op < A_EQ || condnode->op > A_GE) {
        fatal("Bad comparison operator");
    }
    match(T_SEMI, ";");

    postopnode = singlestmt();
    match(T_RPAREN, ")");

    bodenode = compoundstmt();

    tree = mkastnode(A_GLUE, bodenode, NULL, postopnode, 0);
    tree = mkastnode(A_WHILE, condnode, NULL, tree, 0);

    return mkastnode(A_GLUE, preopnode, NULL, tree, 0);
}

static ASTnode *singlestmt() {
    switch (TOKEN.token) {
        case T_PRINT:
            return stmtprint();
        case T_INT:
            declarevar();
            return NULL;
        case T_IDENT:
            return stmtassign();
        case T_IF:
            return stmtif();
        case T_WHILE:
            return stmtwhile();
        case T_FOR:
            return stmtfor();
        default:
            fatald("syntax error, unexpected token", TOKEN.token);
    }
    return NULL;
}

ASTnode *compoundstmt() {
    ASTnode *tree, *left = NULL;
    match(T_LBRACE, "{");

    while (1) {
        tree = singlestmt();

        if (tree && (tree->op == A_PRINT || tree->op == A_ASSIGN)) {
            match(T_SEMI, ";");
        }

        if (tree) {
            if (left) {
                left = mkastnode(A_GLUE, left, NULL, tree, 0);
            } else {
                left = tree;
            }
        }

        if (TOKEN.token == T_RBRACE) {
            match(T_RBRACE, "}");
            return left;
        }
    }
}
