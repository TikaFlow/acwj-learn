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
    gen_ast(node->mid, NO_REG, node->op);
    gen_free_regs();

    if (node->right) {
        cg_jump(lend);
    }
    cg_label(lfalse);
    if (node->right) {
        gen_ast(node->right, NO_REG, node->op);
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

    gen_ast(node->right, NO_REG, node->op);
    gen_free_regs();

    cg_jump(lbegin);
    cg_label(lend);

    return NO_REG;
}

int gen_ast(ASTnode *node, int reg, int parentASTop) {
    int leftreg, rightreg;

    switch (node->op) {
        case A_IF:
            return gen_if_ast(node);
        case A_WHILE:
            return gen_while_ast(node);
        case A_GLUE:
            gen_ast(node->left, NO_REG, node->op);
            gen_free_regs();
            gen_ast(node->right, NO_REG, node->op);
            gen_free_regs();
            return NO_REG;
        case A_FUNCTION:
            cg_func_pre_amble(node->value.id);
            gen_ast(node->left, NO_REG, node->op);
            cg_func_post_amble(node->value.id);
            return NO_REG;
    }

    if (node->left) {
        leftreg = gen_ast(node->left, NO_REG, node->op);
    }
    if (node->right) {
        rightreg = gen_ast(node->right, leftreg, node->op);
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
        case A_EQ:
        case A_NE:
        case A_LT:
        case A_GT:
        case A_LE:
        case A_GE:
            if (parentASTop == A_IF || parentASTop == A_WHILE) {
                return cg_compare_and_jump(node->op, leftreg, rightreg, reg);
            } else {
                return cg_compare_and_set(node->op, leftreg, rightreg);
            }
        case A_INTLIT:
            return cg_load_int(node->value.int_value);
        case A_IDENT:
            return cg_load_sym(node->value.id);
        case A_LVIDENT:
            return cg_store_sym(reg, node->value.id);
        case A_ASSIGN:
            return rightreg;
        case A_PRINT:
            gen_print_int(leftreg);
            gen_free_regs();
            return NO_REG;
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
            return cg_deref(leftreg, node->left->type);
        default:
            fatald("Unknown AST operator", node->op);
    }
    return NO_REG;
}

void gen_pre_amble() {
    cg_pre_amble();
}

void gen_free_regs() {
    cg_free_regs();
}

void gen_print_int(int reg) {
    cg_print_int(reg);
}

void gen_new_sym(int id) {
    cg_new_sym(id);
}

int gen_type_size(int type) {
    return cg_type_size(type);
}
