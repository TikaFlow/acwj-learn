//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

ASTnode *mkastnode(int op, ASTnode *left, ASTnode *mid, ASTnode *right, int intvalue) {

    ASTnode *node = (ASTnode *) malloc(sizeof(ASTnode));

    if (!node) {
        fprintf(stderr, "Unable to malloc in mkastnode()\n");
        exit(1);
    }

    node->op = op;
    node->left = left;
    node->mid = mid;
    node->right = right;
    node->value.intvalue = intvalue;
    return node;
}

ASTnode *mkastleaf(int op, int intvalue) {
    return mkastnode(op, NULL, NULL, NULL, intvalue);
}

ASTnode *mkastunary(int op, ASTnode *left, int intvalue) {
    return mkastnode(op, left, NULL, NULL, intvalue);
}
