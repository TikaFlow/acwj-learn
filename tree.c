//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

ASTnode *make_ast_node(int op, int type, ASTnode *left, ASTnode *mid, ASTnode *right, Symbol *sym, long int_value) {

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
    node->sym = sym;
    node->int_value = int_value;
    return node;
}

ASTnode *make_ast_leaf(int op, int type, Symbol *sym, long int_value) {
    return make_ast_node(op, type, NULL, NULL, NULL, sym, int_value);
}

ASTnode *make_ast_unary(int op, int type, ASTnode *left, Symbol *sym, long int_value) {
    return make_ast_node(op, type, left, NULL, NULL, sym, int_value);
}

static int gendumplabel() {
    static int id = 1;
    return id++;
}

void dump_ast(ASTnode *node, int label, int level) {
    int Lfalse, Lstart, Lend;


    switch (node->op) {
        case A_IF:
            Lfalse = gendumplabel();
            for (int i = 0; i < level; i++) fprintf(stdout, " ");
            fprintf(stdout, "A_IF");
            if (node->right) {
                Lend = gendumplabel();
                fprintf(stdout, ", end L%d", Lend);
            }
            fprintf(stdout, "\n");
            dump_ast(node->left, Lfalse, level + 2);
            dump_ast(node->mid, NO_LABEL, level + 2);
            if (node->right) dump_ast(node->right, NO_LABEL, level + 2);
            return;
        case A_WHILE:
            Lstart = gendumplabel();
            for (int i = 0; i < level; i++) fprintf(stdout, " ");
            fprintf(stdout, "A_WHILE, start L%d\n", Lstart);
            Lend = gendumplabel();
            dump_ast(node->left, Lend, level + 2);
            dump_ast(node->right, NO_LABEL, level + 2);
            return;
    }

    // Reset level to -2 for A_GLUE
    if (node->op == A_GLUE) level = -2;

    // General AST node handling
    if (node->left) dump_ast(node->left, NO_LABEL, level + 2);
    if (node->right) dump_ast(node->right, NO_LABEL, level + 2);


    for (int i = 0; i < level; i++) fprintf(stdout, " ");
    switch (node->op) {
        case A_GLUE:
            fprintf(stdout, "\n\n");
            return;
        case A_FUNCTION:
            fprintf(stdout, "A_FUNCTION %s\n", node->sym->name);
            return;
        case A_ADD:
            fprintf(stdout, "A_ADD\n");
            return;
        case A_SUBTRACT:
            fprintf(stdout, "A_SUBTRACT\n");
            return;
        case A_MULTIPLY:
            fprintf(stdout, "A_MULTIPLY\n");
            return;
        case A_DIVIDE:
            fprintf(stdout, "A_DIVIDE\n");
            return;
        case A_EQ:
            fprintf(stdout, "A_EQ\n");
            return;
        case A_NE:
            fprintf(stdout, "A_NE\n");
            return;
        case A_LT:
            fprintf(stdout, "A_LE\n");
            return;
        case A_GT:
            fprintf(stdout, "A_GT\n");
            return;
        case A_LE:
            fprintf(stdout, "A_LE\n");
            return;
        case A_GE:
            fprintf(stdout, "A_GE\n");
            return;
        case A_INTLIT:
            fprintf(stdout, "A_INTLIT %ld\n", node->int_value);
            return;
        case A_IDENT:
            if (node->rvalue)
                fprintf(stdout, "A_IDENT rval %s\n", node->sym->name);
            else
                fprintf(stdout, "A_IDENT %s\n", node->sym->name);
            return;
        case A_ASSIGN:
            fprintf(stdout, "A_ASSIGN\n");
            return;
        case A_WIDEN:
            fprintf(stdout, "A_WIDEN\n");
            return;
        case A_RETURN:
            fprintf(stdout, "A_RETURN\n");
            return;
        case A_FUNCCALL:
            fprintf(stdout, "A_FUNCCALL %s\n", node->sym->name);
            return;
        case A_ADDR:
            fprintf(stdout, "A_ADDR %s\n", node->sym->name);
            return;
        case A_DEREF:
            if (node->rvalue)
                fprintf(stdout, "A_DEREF rval\n");
            else
                fprintf(stdout, "A_DEREF\n");
            return;
        case A_SCALE:
            fprintf(stdout, "A_SCALE %d\n", node->size);
            return;
        default:
            fatals("Unknown dump_ast operator", get_name(V_PTYPE, node->op));
    }
}
