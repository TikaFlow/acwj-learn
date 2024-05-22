//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static ASTnode *stmtprint() {
    match(T_PRINT, "print");
    ASTnode *tree = binexpr(0);
    tree = mkastunary(A_PRINT, tree, 0);

    match(T_SEMI, ";");
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

    match(T_SEMI, ";");
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

ASTnode *compoundstmt() {
    ASTnode *tree, *left = NULL;
    match(T_LBRACE, "{");

    while (1) {
        switch (TOKEN.token) {
            case T_PRINT:
                tree = stmtprint();
                break;
            case T_INT:
                declarevar();
                tree = NULL;
                break;
            case T_IDENT:
                tree = stmtassign();
                break;
            case T_IF:
                tree = stmtif();
                break;
            case T_RBRACE:
                match(T_RBRACE, "}");
                return left;
            default:
                fatald("syntax error, unexpected token:", TOKEN.token);
        }

        if (tree) {
            if (left) {
                left = mkastnode(A_GLUE, left, NULL, tree, 0);
            } else {
                left = tree;
            }
        }
    }
}
