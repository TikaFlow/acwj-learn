//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

int interpretAST(ASTnode *ast) {
    int leftvalue, rightvalue;

    if (ast->left) {
        leftvalue = interpretAST(ast->left);
    }

    if (ast->right) {
        rightvalue = interpretAST(ast->right);
    }

    switch (ast->op) {
        case A_ADD:
            return leftvalue + rightvalue;

        case A_SUBTRACT:
            return leftvalue - rightvalue;

        case A_MULTIPLY:
            return leftvalue * rightvalue;

        case A_DIVIDE:
            return leftvalue / rightvalue;

        case A_INTLIT:
            return ast->intvalue;
        default:
            fprintf(stderr, "Unknown AST operator %d\n", ast->op);
            exit(1);
    }
}
