//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

int gen_label() {
    static int id = 0;
    return id++;
}

static int gen_if_ast(ASTnode *node) {
    int lend, lfalse = gen_label();
    if (node->right) {
        lend = gen_label();
    }

    gen_ast(node->left, lfalse, node->op);
    gen_free_regs();
    gen_ast(node->mid, NO_LABEL, node->op);
    gen_free_regs();

    if (node->right) {
        cg_jump(lend);
    }
    cg_label(lfalse);
    if (node->right) {
        gen_ast(node->right, NO_LABEL, node->op);
        gen_free_regs();
        cg_label(lend);
    }
    return NO_REG;
}

static int gen_while_ast(ASTnode *node) {
    int lbegin = gen_label(), lend = gen_label();
    cg_label(lbegin);

    gen_ast(node->left, lend, node->op);
    gen_free_regs();

    gen_ast(node->right, NO_LABEL, node->op);
    gen_free_regs();

    cg_jump(lbegin);
    cg_label(lend);

    return NO_REG;
}

int gen_ast(ASTnode *node, int label, int parent_op) {
    int leftreg, rightreg;

    switch (node->op) {
        case A_IF:
            return gen_if_ast(node);
        case A_WHILE:
            return gen_while_ast(node);
        case A_GLUE:
            gen_ast(node->left, NO_LABEL, node->op);
            gen_free_regs();
            gen_ast(node->right, NO_LABEL, node->op);
            gen_free_regs();
            return NO_REG;
        case A_FUNCTION:
            cg_func_pre_amble(node->value.id);
            gen_ast(node->left, NO_LABEL, node->op);
            cg_func_post_amble(node->value.id);
            return NO_REG;
    }

    if (node->left) {
        leftreg = gen_ast(node->left, NO_LABEL, node->op);
    }
    if (node->right) {
        rightreg = gen_ast(node->right, NO_LABEL, node->op);
    }

    switch (node->op) {
        case A_ADD:
            return cg_add(leftreg, rightreg);
        case A_SUBTRACT:
            return cg_sub(leftreg, rightreg);
        case A_MULTIPLY:
            return cg_mul(leftreg, rightreg);
        case A_DIVIDE:
            return cg_div(leftreg, rightreg);
        case A_AND:
            return cg_and(leftreg, rightreg);
        case A_OR:
            return cg_or(leftreg, rightreg);
        case A_XOR:
            return cg_xor(leftreg, rightreg);
        case A_LSHIFT:
            return cg_sal(leftreg, rightreg);
        case A_RSHIFT:
            return cg_sar(leftreg, rightreg);
        case A_EQ:
        case A_NE:
        case A_LT:
        case A_GT:
        case A_LE:
        case A_GE:
            if (parent_op == A_IF || parent_op == A_WHILE) {
                return cg_compare_and_jump(node->op, leftreg, rightreg, label);
            }
            return cg_compare_and_set(node->op, leftreg, rightreg);
        case A_INTLIT:
            return cg_load_int(node->value.int_value);
        case A_STRLIT:
            return cg_load_str(node->value.id);
        case A_IDENT:
            if (node->rvalue || parent_op == A_DEREF) {
                if (SYM_TAB[node->value.id].class != C_GLOBAL) {
                    return cg_load_local_sym(node->value.id, node->op);
                } else {
                    return cg_load_global_sym(node->value.id, node->op);
                }
            }
            return NO_REG;
        case A_ASSIGN:
            switch (node->right->op) {
                case A_IDENT:
                    if (SYM_TAB[node->right->value.id].class != C_GLOBAL) {
                        return cg_store_local_sym(leftreg, node->right->value.id);
                    } else {
                        return cg_store_global_sym(leftreg, node->right->value.id);
                    }
                case A_DEREF:
                    return cg_store_deref(leftreg, rightreg, node->right->type);
                default:
                    fatald("Can't A_ASSIGN in gen_ast, op", node->op);
            }
        case A_WIDEN:
            return cg_widen(leftreg, node->left->type, node->type);
        case A_RETURN:
            cg_return(leftreg, FUNC_ID);
            return NO_REG;
        case A_FUNCCALL:
            return cg_call(leftreg, node->value.id);
        case A_ADDR:
            return cg_address(node->value.id);
        case A_DEREF:
            if (node->rvalue) {
                return cg_deref(leftreg, node->left->type);
            }
            return leftreg;
        case A_SCALE:
            switch (node->value.size) {
                case 2:
                    return cg_sal_n(leftreg, 1);
                case 4:
                    return cg_sal_n(leftreg, 2);
                case 8:
                    return cg_sal_n(leftreg, 3);
                default:
                    rightreg = cg_load_int(node->value.size);
                    return cg_mul(leftreg, rightreg);
            }
        case A_POSTINC:
        case A_POSTDEC:
            if (SYM_TAB[node->value.id].class != C_GLOBAL) {
                return cg_load_local_sym(node->value.id, node->op);
            } else {
                return cg_load_global_sym(node->value.id, node->op);
            }
        case A_PREINC:
        case A_PREDEC:
            if (SYM_TAB[node->left->value.id].class != C_GLOBAL) {
                return cg_load_local_sym(node->left->value.id, node->op);
            } else {
                return cg_load_global_sym(node->left->value.id, node->op);
            }
        case A_NEGATE:
            return cg_negate(leftreg);
        case A_INVERT:
            return cg_invert(leftreg);
        case A_LOGNOT:
            return cg_lognot(leftreg);
        case A_TOBOOL:
            return cg_tobool(leftreg, parent_op, label);
        default:
            fatald("Unknown AST operator", node->op);
    }
    return NO_REG;
}

void gen_pre_amble() {
    cg_pre_amble();
}

void gen_post_amble() {
    cg_post_amble();
}

void gen_free_regs() {
    cg_free_regs();
}

void gen_new_sym(int id) {
    cg_new_sym(id);
}

int gen_new_str(char *str) {
    int l = gen_label();
    cg_new_str(l, str);
    return l;
}

int gen_type_size(int type) {
    return cg_type_size(type);
}
