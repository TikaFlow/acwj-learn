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

static int gendumplabel() {
    static int id = 1;
    return id++;
}

void dump_ast(ASTnode *node, int label, int level) {
    int Lfalse, Lstart, Lend;

    switch (node->op) {
        case A_IF:
            Lfalse = gendumplabel();
            for (int i = 0; i < level; i++) {
                fprintf(stdout, " ");
            }
            fprintf(stdout, "A_IF");
            if (node->right) {
                Lend = gendumplabel();
                fprintf(stdout, ", end L%d", Lend);
            }
            fprintf(stdout, "\n");
            dump_ast(node->left, Lfalse, level + 2);
            dump_ast(node->mid, NO_LABEL, level + 2);
            if (node->right) {
                dump_ast(node->right, NO_LABEL, level + 2);
            }
            return;
        case A_WHILE:
            Lstart = gendumplabel();
            for (int i = 0; i < level; i++) {
                fprintf(stdout, " ");
            }
            fprintf(stdout, "A_WHILE, start L%d\n", Lstart);
            Lend = gendumplabel();
            dump_ast(node->left, Lend, level + 2);
            dump_ast(node->right, NO_LABEL, level + 2);
            return;
    }

    // Reset level to -2 for A_GLUE
    if (node->op == A_GLUE) {
        level = -2;
    }

    // General AST node handling
    if (node->left) {
        dump_ast(node->left, NO_LABEL, level + 2);
    }
    if (node->right) {
        dump_ast(node->right, NO_LABEL, level + 2);
    }


    for (int i = 0; i < level; i++) {
        fprintf(stdout, " ");
    }
    switch (node->op) {
        case A_GLUE:
            fprintf(stdout, "\n");
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
        case A_STRLIT:
            fprintf(stdout, "A_STRLIT rval label L%d\n", (int) node->int_value);
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
        case A_PREINC:
            fprintf(stdout, "A_PREINC %s\n", node->sym->name);
            return;
        case A_PREDEC:
            fprintf(stdout, "A_PREDEC %s\n", node->sym->name);
            return;
        case A_POSTINC:
            fprintf(stdout, "A_POSTINC\n");
            return;
        case A_POSTDEC:
            fprintf(stdout, "A_POSTDEC\n");
            return;
        case A_NEGATE:
            fprintf(stdout, "A_NEGATE\n");
            return;
        case A_BREAK:
            fprintf(stdout, "A_BREAK\n");
            return;
        case A_CONTINUE:
            fprintf(stdout, "A_CONTINUE\n");
            return;
        case A_CASE:
            fprintf(stdout, "A_CASE %d\n", (int) node->int_value);
            return;
        case A_DEFAULT:
            fprintf(stdout, "A_DEFAULT\n");
            return;
        case A_SWITCH:
            fprintf(stdout, "A_SWITCH\n");
            return;
        case A_CAST:
            fprintf(stdout, "A_CAST %d\n", node->type);
            return;
        case A_ASPLUS:
            fprintf(stdout, "A_ASPLUS\n");
            return;
        case A_ASMINUS:
            fprintf(stdout, "A_ASMINUS\n");
            return;
        case A_ASSTAR:
            fprintf(stdout, "A_ASSTAR\n");
            return;
        case A_ASSLASH:
            fprintf(stdout, "A_ASSLASH\n");
            return;
        case A_TOBOOL:
            fprintf(stdout, "A_TOBOOL\n");
            return;
        case A_LOGOR:
            fprintf(stdout, "A_LOGOR\n");
            return;
        case A_LOGAND:
            fprintf(stdout, "A_LOGAND\n");
            return;
        case A_AND:
            fprintf(stdout, "A_AND\n");
            return;
        case A_ASMOD:
            fprintf(stdout, "A_ASMOD\n");
            return;
        case A_INVERT:
            fprintf(stdout, "A_INVERT\n");
            return;
        case A_LOGNOT:
            fprintf(stdout, "A_LOGNOT\n");
            return;
        case A_LSHIFT:
            fprintf(stdout, "A_LSHIFT\n");
            return;
        case A_MOD:
            fprintf(stdout, "A_MOD\n");
            return;
        case A_OR:
            fprintf(stdout, "A_OR\n");
            return;
        case A_RSHIFT:
            fprintf(stdout, "A_RSHIFT\n");
            return;
        case A_TERNARY:
            fprintf(stdout, "A_TERNARY\n");
            return;
        case A_XOR:
            fprintf(stdout, "A_XOR\n");
            return;
        default:
            fatals("Unknown dump_ast operator", get_name(V_PTYPE, node->op));
    }
}
