//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static int genAST(ASTnode *node) {
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

void generatecode(ASTnode *node) {
    int reg;

    cgpreamble();
    reg = genAST(node);
    cgprintint(reg);
    cgpostamble();
}
