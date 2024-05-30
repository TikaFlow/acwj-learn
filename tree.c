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

static int gendumplabel() {
    static int id = 1;
    return id++;
}

void dump_ast(ASTnode *n, int label, int level) {
    int Lfalse, Lstart, Lend;


    switch (n->op) {
        case A_IF:
            Lfalse = gendumplabel();
            for (int i=0; i < level; i++) fprintf(stdout, " ");
            fprintf(stdout, "A_IF");
            if (n->right) { Lend = gendumplabel();
                fprintf(stdout, ", end L%d", Lend);
            }
            fprintf(stdout, "\n");
            dump_ast(n->left, Lfalse, level+2);
            dump_ast(n->mid, NO_LABEL, level+2);
            if (n->right) dump_ast(n->right, NO_LABEL, level+2);
            return;
        case A_WHILE:
            Lstart = gendumplabel();
            for (int i=0; i < level; i++) fprintf(stdout, " ");
            fprintf(stdout, "A_WHILE, start L%d\n", Lstart);
            Lend = gendumplabel();
            dump_ast(n->left, Lend, level+2);
            dump_ast(n->right, NO_LABEL, level+2);
            return;
    }

    // Reset level to -2 for A_GLUE
    if (n->op==A_GLUE) level= -2;

    // General AST node handling
    if (n->left) dump_ast(n->left, NO_LABEL, level+2);
    if (n->right) dump_ast(n->right, NO_LABEL, level+2);


    for (int i=0; i < level; i++) fprintf(stdout, " ");
    switch (n->op) {
        case A_GLUE:
            fprintf(stdout, "\n\n"); return;
        case A_FUNCTION:
            fprintf(stdout, "A_FUNCTION %s\n", SYM_TAB[n->value.id].name); return;
        case A_ADD:
            fprintf(stdout, "A_ADD\n"); return;
        case A_SUBTRACT:
            fprintf(stdout, "A_SUBTRACT\n"); return;
        case A_MULTIPLY:
            fprintf(stdout, "A_MULTIPLY\n"); return;
        case A_DIVIDE:
            fprintf(stdout, "A_DIVIDE\n"); return;
        case A_EQ:
            fprintf(stdout, "A_EQ\n"); return;
        case A_NE:
            fprintf(stdout, "A_NE\n"); return;
        case A_LT:
            fprintf(stdout, "A_LE\n"); return;
        case A_GT:
            fprintf(stdout, "A_GT\n"); return;
        case A_LE:
            fprintf(stdout, "A_LE\n"); return;
        case A_GE:
            fprintf(stdout, "A_GE\n"); return;
        case A_INTLIT:
            fprintf(stdout, "A_INTLIT %ld\n", n->value.int_value); return;
        case A_IDENT:
            if (n->rvalue)
                fprintf(stdout, "A_IDENT rval %s\n", SYM_TAB[n->value.id].name);
            else
                fprintf(stdout, "A_IDENT %s\n", SYM_TAB[n->value.id].name);
            return;
        case A_ASSIGN:
            fprintf(stdout, "A_ASSIGN\n"); return;
        case A_WIDEN:
            fprintf(stdout, "A_WIDEN\n"); return;
        case A_RETURN:
            fprintf(stdout, "A_RETURN\n"); return;
        case A_FUNCCALL:
            fprintf(stdout, "A_FUNCCALL %s\n", SYM_TAB[n->value.id].name); return;
        case A_ADDR:
            fprintf(stdout, "A_ADDR %s\n", SYM_TAB[n->value.id].name); return;
        case A_DEREF:
            if (n->rvalue)
                fprintf(stdout, "A_DEREF rval\n");
            else
                fprintf(stdout, "A_DEREF\n");
            return;
        case A_SCALE:
            fprintf(stdout, "A_SCALE %d\n", n->value.size); return;
        default:
            fatald("Unknown dump_ast operator", n->op);
    }
}
