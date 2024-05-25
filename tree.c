//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

ASTnode *make_ast_node(int op, int type, ASTnode *left, ASTnode *mid, ASTnode *right, long int_value) {

    ASTnode *node = (ASTnode *) malloc(sizeof(ASTnode));

    if (!node) {
        fprintf(stderr, "Unable to malloc in make_ast_node()\n");
        exit(1);
    }

    node->op = op;
    node->type = type;
    node->left = left;
    node->mid = mid;
    node->right = right;
    node->value.int_value = int_value;
    return node;
}

ASTnode *make_ast_leaf(int op, int type, long int_value) {
    return make_ast_node(op, type, NULL, NULL, NULL, int_value);
}

ASTnode *make_ast_unary(int op, int type, ASTnode *left, long int_value) {
    return make_ast_node(op, type, left, NULL, NULL, int_value);
}
