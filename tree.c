//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

ASTnode *make_ast_node(int op, int type, Symbol *ctype, ASTnode *left, ASTnode *mid, ASTnode *right, Symbol *sym, long
int_value) {

    ASTnode *node = (ASTnode *) malloc(sizeof(ASTnode));

    if (!node) {
        fprintf(stderr, "Unable to malloc in make_ast_node()\n");
        exit(1);
    }

    node->op = op;
    node->type = type;
    node->ctype = ctype;
    node->left = left;
    node->mid = mid;
    node->right = right;
    node->sym = sym;
    node->int_value = int_value;
    node->line = 0;
    return node;
}

ASTnode *make_ast_unary(int op, int type, Symbol *ctype, ASTnode *left, Symbol *sym, long int_value) {
    return make_ast_node(op, type, ctype, left, NULL, NULL, sym, int_value);
}

ASTnode *make_ast_leaf(int op, int type, Symbol *ctype, Symbol *sym, long int_value) {
    return make_ast_unary(op, type, ctype, NULL, sym, int_value);
}
