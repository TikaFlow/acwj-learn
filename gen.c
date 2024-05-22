//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

int genAST(ASTnode *node, int reg) {
    int leftreg, rightreg;

    if (node->left) {
        leftreg = genAST(node->left, -1);
    }
    if (node->right) {
        rightreg = genAST(node->right, leftreg);
    }

    switch (node->op) {
        case A_ADD:
            return cgadd(leftreg, rightreg);
        case A_SUBTRACT:
            return cgsub(leftreg, rightreg);
        case A_MULTIPLY:
            return cgmul(leftreg, rightreg);
        case A_DIVIDE:
            return cgdiv(leftreg, rightreg);
        case A_EQ:
            return cgequal(leftreg, rightreg);
        case A_NE:
            return cgnotequal(leftreg, rightreg);
        case A_LT:
            return cglessthan(leftreg, rightreg);
        case A_GT:
            return cggreaterthan(leftreg, rightreg);
        case A_LE:
            return cglessequal(leftreg, rightreg);
        case A_GE:
            return cggreaterequal(leftreg, rightreg);
        case A_INTLIT:
            return cgloadint(node->value.intvalue);
        case A_IDENT:
            return cgloadglob(SYM_TAB[node->value.id].name);
        case A_LVIDENT:
            return cgstorglob(reg, SYM_TAB[node->value.id].name);
        case A_ASSIGN:
            return rightreg;
        default:
            fatald("Unknown AST operator", node->op);
    }
    return -1;
}

void genpreamble() {
    cgpreamble();
}

void genpostamble() {
    cgpostamble();
}

void genfreeregs() {
    cgfreeregs();
}

void genprintint(int reg) {
    cgprintint(reg);
}

void genglobsym(char *s) {
    cgglobsym(s);
}
