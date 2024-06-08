//
// Created by tika on 24-6-8.
//

#include "data.h"
#include "decl.h"

static ASTnode *fold1(ASTnode *tree) {
    long val = tree->left->int_value;

    switch (tree->op) {
        case A_WIDEN:
            break;
        case A_INVERT:
            val = ~val;
            break;
        case A_LOGNOT:
            val = !val;
            break;
        case A_NEGATE:
            val = -val;
            break;
        default:
            return tree;
    }

    return make_ast_leaf(A_INTLIT, tree->type, NULL, val);
}

static ASTnode *fold2(ASTnode *tree) {
    long val, left_val = tree->left->int_value, right_val = tree->right->int_value;

    switch (tree->op) {
        case A_ADD:
            val = left_val + right_val;
            break;
        case A_SUBTRACT:
            val = left_val - right_val;
            break;
        case A_MULTIPLY:
            val = left_val * right_val;
            break;
        case A_DIVIDE:
            if (right_val == 0) {
                return tree;
            }
            val = left_val / right_val;
            break;
        default:
            return tree;
    }

    return make_ast_leaf(A_INTLIT, tree->type, NULL, val);
}

static ASTnode *fold(ASTnode *tree) {
    if (!tree) {
        return NULL;
    }

    tree->left = fold(tree->left);
    tree->right = fold(tree->right);

    if (tree->left && tree->left->op == A_INTLIT) {
        if (tree->right && tree->right->op == A_INTLIT) {
            tree = fold2(tree);
        } else {
            tree = fold1(tree);
        }
    }

    return tree;
}

ASTnode *optimize(ASTnode *tree) {
    return fold(tree);
}
