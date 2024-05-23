//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static int label() {
    static int id = 0;
    return id++;
}

static int genifAST(ASTnode *node) {
    int lend, lfalse = label();
    if (node->right) {
        lend = label();
    }

    genAST(node->left, lfalse, node->op);
    genfreeregs();
    genAST(node->mid, NO_REG, node->op);
    genfreeregs();

    if (node->right) {
        cgjump(lend);
    }
    cglabel(lfalse);
    if (node->right) {
        genAST(node->right, NO_REG, node->op);
        genfreeregs();
        cglabel(lend);
    }
    return NO_REG;
}

static int genwhileAST(ASTnode *node) {
    int lbegin = label(), lend = label();
    cglabel(lbegin);

    genAST(node->left, lend, node->op);
    genfreeregs();

    genAST(node->right, NO_REG, node->op);
    genfreeregs();

    cgjump(lbegin);
    cglabel(lend);

    return NO_REG;
}

int genAST(ASTnode *node, int reg, int parentASTop) {
    int leftreg, rightreg;

    switch (node->op) {
        case A_IF:
            return genifAST(node);
        case A_WHILE:
            return genwhileAST(node);
        case A_GLUE:
            genAST(node->left, NO_REG, node->op);
            genfreeregs();
            genAST(node->right, NO_REG, node->op);
            genfreeregs();
            return NO_REG;
        case A_FUNCTION:
            cgfuncpreamble(SYM_TAB[node->value.id].name);
            genAST(node->left, NO_REG, node->op);
            cgfuncpostamble();
            return NO_REG;
    }

    if (node->left) {
        leftreg = genAST(node->left, NO_REG, node->op);
    }
    if (node->right) {
        rightreg = genAST(node->right, leftreg, node->op);
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
        case A_NE:
        case A_LT:
        case A_GT:
        case A_LE:
        case A_GE:
            if (parentASTop == A_IF || parentASTop == A_WHILE) {
                return cgcompare_and_jump(node->op, leftreg, rightreg, reg);
            } else {
                return cgcompare_and_set(node->op, leftreg, rightreg);
            }
        case A_INTLIT:
            return cgloadint(node->value.intvalue);
        case A_IDENT:
            return cgloadglob(SYM_TAB[node->value.id].name);
        case A_LVIDENT:
            return cgstorglob(reg, SYM_TAB[node->value.id].name);
        case A_ASSIGN:
            return rightreg;
        case A_PRINT:
            genprintint(leftreg);
            genfreeregs();
            return NO_REG;
        default:
            fatald("Unknown AST operator", node->op);
    }
    return NO_REG;
}

void genpreamble() {
    cgpreamble();
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
