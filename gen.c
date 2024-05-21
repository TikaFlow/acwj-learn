//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

int genAST(ASTnode *node) {
    int leftreg, rightreg;

    if (node->left) {
        leftreg = genAST(node->left);
    }
    if (node->right) {
        rightreg = genAST(node->right);
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
        case A_INTLIT:
            return cgload(node->intvalue);

        default:
            fprintf(stderr, "Unknown AST operator: %d\n", node->op);
            exit(1);
    }
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
