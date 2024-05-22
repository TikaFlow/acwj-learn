//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void stmtprint() {
    ASTnode *tree;
    int reg;

    match(T_PRINT, "print");
    tree = binexpr(0);
    reg = genAST(tree, -1);
    genprintint(reg);
    genfreeregs();

    semi();
}

void stmtassign() {
    ASTnode *left, *right, *tree;
    int id;

    ident();
    if ((id = findglob(TEXT)) < 0) {
        fatals("undeclared variable:", TEXT);
    }
    right = mkastleaf(A_LVIDENT, id);

    match(T_ASSIGN, "=");
    left = binexpr(0);

    tree = mkastnode(A_ASSIGN, left, right, 0);
    genAST(tree, -1);
    genfreeregs();

    semi();
}

void statements() {
    while (1) {
        switch (TOKEN.token) {
            case T_PRINT:
                stmtprint();
                break;
            case T_INT:
                declarevar();
                break;
            case T_IDENT:
                stmtassign();
                break;
            case T_EOF:
                return;
            default:
                fatald("syntax error, unexpected token:", TOKEN.token);
        }
    }
}
