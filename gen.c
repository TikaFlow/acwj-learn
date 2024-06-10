//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

int gen_label() {
    static int id = 0; // TODO support closures later
    return id++;
}

static int gen_if_ast(ASTnode *node, int start_label, int end_label) {
    int lend, lfalse = gen_label();
    int exp_reg, reg = cg_alloc_register();
    if (node->right) {
        lend = gen_label();
    }

    // condition statements
    // when false, jump to lfalse
    gen_ast(node->left, lfalse, NO_LABEL, NO_LABEL, node->op);
    gen_free_regs(NO_REG);
    // true statements
    // when true, no jump
    exp_reg = gen_ast(node->mid, NO_LABEL, start_label, end_label, node->op);
    // when is if node, exp_reg is NO_REG(-1), so DO NOT need to move to reg
    if (node->op == A_TERNARY) {
        cg_mov_reg(exp_reg, reg);
    }
    gen_free_regs(reg);

    if (node->right) {
        cg_jump(lend);
    }
    cg_label(lfalse);
    if (node->right) {
        // else statements
        exp_reg = gen_ast(node->right, NO_LABEL, start_label, end_label, node->op);
        if (node->op == A_TERNARY) {
            cg_mov_reg(exp_reg, reg);
        }
        gen_free_regs(reg);
        cg_label(lend);
    }

    return reg;
}

static int gen_while_ast(ASTnode *node) {
    int lbegin = gen_label(), lend = gen_label();
    cg_label(lbegin);

    gen_ast(node->left, lend, lbegin, lend, node->op);
    gen_free_regs(NO_REG);

    gen_ast(node->right, NO_LABEL, lbegin, lend, node->op);
    gen_free_regs(NO_REG);

    cg_jump(lbegin);
    cg_label(lend);

    return NO_REG;
}

static int gen_switch_ast(ASTnode *node) {
    ASTnode *case_node;
    int *case_val, *case_label, end_label, i, reg, dft_label;
    case_val = (int *) malloc((node->int_value) * sizeof(int));
    case_label = (int *) malloc((node->int_value) * sizeof(int));

    end_label = gen_label();
    dft_label = end_label;

    // parse case statements
    for (i = 0, case_node = node->right; case_node; i++, case_node = case_node->right) {
        case_val[i] = (int) case_node->int_value;
        case_label[i] = gen_label();

        if (case_node->op == A_DEFAULT) {
            dft_label = case_label[i];
        }
    }

    // gen condition statements
    reg = gen_ast(node->left, NO_LABEL, NO_LABEL, NO_LABEL, A_NONE);
    // gen switch table
    cg_switch(reg, (int) node->int_value, case_label, case_val, dft_label);
    gen_free_regs(NO_REG);

    // gen case statements
    for (i = 0, case_node = node->right; case_node; i++, case_node = case_node->right) {
        cg_label(case_label[i]);
        gen_ast(case_node->left, NO_LABEL, NO_LABEL, end_label, A_NONE);
        gen_free_regs(NO_REG);
    }

    cg_label(end_label);

    return NO_REG;
}

static int gen_func_call(ASTnode *node) {
    ASTnode *glue = node;
    int reg, args_num = 0;

    // walk through the arguments
    while ((glue = glue->left)) {
        reg = gen_ast(glue->right, NO_LABEL, NO_LABEL, NO_LABEL, glue->op);
        cg_copy_arg(reg, glue->size);

        if (!args_num) {
            args_num = glue->size + 1;
        }

        gen_free_regs(NO_REG);
    }

    return cg_call(node->sym, args_num);
}

int gen_ast(ASTnode *node, int if_label, int start_label, int end_label, int parent_op) {
    int leftreg, rightreg;

    if (!node) {
        return NO_REG;
    }

    switch (node->op) {
        case A_IF:
            return gen_if_ast(node, start_label, end_label);
        case A_WHILE:
            return gen_while_ast(node);
        case A_SWITCH:
            return gen_switch_ast(node);
        case A_FUNCCALL:
            return gen_func_call(node);
        case A_TERNARY:
            return gen_if_ast(node, NO_LABEL, NO_LABEL);
        case A_GLUE:
            gen_ast(node->left, if_label, start_label, end_label, node->op);
            gen_free_regs(NO_REG);
            gen_ast(node->right, if_label, start_label, end_label, node->op);
            gen_free_regs(NO_REG);
            return NO_REG;
        case A_FUNCTION:
            cg_func_pre_amble(node->sym);
            gen_ast(node->left, NO_LABEL, NO_LABEL, NO_LABEL, node->op);
            cg_func_post_amble(node->sym);
            return NO_REG;
    }

    if (node->left) {
        leftreg = gen_ast(node->left, NO_LABEL, NO_LABEL, NO_LABEL, node->op);
    }
    if (node->right) {
        rightreg = gen_ast(node->right, NO_LABEL, NO_LABEL, NO_LABEL, node->op);
    }

    switch (node->op) {
        case A_NOP:
            return NO_REG;
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
            if (parent_op == A_IF || parent_op == A_WHILE || parent_op == A_TERNARY) {
                return cg_compare_and_jump(node->op, leftreg, rightreg, if_label);
            }
            return cg_compare_and_set(node->op, leftreg, rightreg);
        case A_INTLIT:
            return cg_load_int(node->int_value);
        case A_STRLIT:
            return cg_load_str(node->int_value);
        case A_IDENT:
            if (node->rvalue || parent_op == A_DEREF) {
                if (node->sym->class == C_GLOBAL || node->sym->class == C_STATIC) {
                    return cg_load_global_sym(node->sym, node->op);
                } else {
                    return cg_load_local_sym(node->sym, node->op);
                }
            }
            return NO_REG;
        case A_ASPLUS:
        case A_ASMINUS:
        case A_ASSTAR:
        case A_ASSLASH:
            switch (node->op) {
                case A_ASPLUS:
                    leftreg = cg_add(leftreg, rightreg);
                    break;
                case A_ASMINUS:
                    leftreg = cg_sub(leftreg, rightreg);
                    break;
                case A_ASSTAR:
                    leftreg = cg_mul(leftreg, rightreg);
                    break;
                case A_ASSLASH:
                    leftreg = cg_div(leftreg, rightreg);
                    break;
            }
            node->right = node->left;
        case A_ASSIGN:
            switch (node->right->op) {
                case A_IDENT:
                    if (node->right->sym->class == C_GLOBAL || node->right->sym->class == C_STATIC) {
                        return cg_store_global_sym(leftreg, node->right->sym);
                    } else {
                        return cg_store_local_sym(leftreg, node->right->sym);
                    }
                case A_DEREF:
                    return cg_store_deref(leftreg, rightreg, node->right->type);
                default:
                    fatals("Can't A_ASSIGN in gen_ast, op", get_name(V_PTYPE, node->op));
            }
        case A_WIDEN:
            return cg_widen(leftreg, node->left->type, node->type);
        case A_RETURN:
            return cg_return(leftreg, FUNC_PTR);
        case A_ADDR:
            return cg_address(node->sym);
        case A_DEREF:
            if (node->rvalue) {
                return cg_deref(leftreg, node->left->type);
            }
            return leftreg;
        case A_SCALE:
            switch (node->size) {
                case 2:
                    return cg_sal_n(leftreg, 1);
                case 4:
                    return cg_sal_n(leftreg, 2);
                case 8:
                    return cg_sal_n(leftreg, 3);
                default:
                    rightreg = cg_load_int(node->size);
                    return cg_mul(leftreg, rightreg);
            }
        case A_POSTINC:
        case A_POSTDEC:
            if (node->sym->class == C_GLOBAL || node->sym->class == C_STATIC) {
                return cg_load_global_sym(node->sym, node->op);
            } else {
                return cg_load_local_sym(node->sym, node->op);
            }
        case A_PREINC:
        case A_PREDEC:
            if (node->left->sym->class == C_GLOBAL || node->left->sym->class == C_STATIC) {
                return cg_load_global_sym(node->left->sym, node->op);
            } else {
                return cg_load_local_sym(node->left->sym, node->op);
            }
        case A_NEGATE:
            return cg_negate(leftreg);
        case A_INVERT:
            return cg_invert(leftreg);
        case A_LOGNOT:
            return cg_lognot(leftreg);
        case A_TOBOOL:
            return cg_tobool(leftreg, parent_op, if_label);
        case A_LOGOR:
            return cg_logor(leftreg, rightreg);
        case A_LOGAND:
            return cg_logand(leftreg, rightreg);
        case A_BREAK:
            return cg_jump(end_label);
        case A_CONTINUE:
            return cg_jump(start_label);
        case A_CAST:
            return leftreg;
        default:
            fatals("Unknown AST operator", get_name(V_PTYPE, node->op));
    }
    return NO_REG;
}

void gen_pre_amble() {
    cg_pre_amble();
}

void gen_post_amble() {
    cg_post_amble();
}

void gen_free_regs(int keep_reg) {
    cg_free_regs(keep_reg);
}

void gen_new_sym(Symbol *sym) {
    cg_new_sym(sym);
}

int gen_new_str(char *str) {
    int label = gen_label();
    cg_new_str(label, str);
    return label;
}

int gen_type_size(int type) {
    return cg_type_size(type);
}

int gen_align(int type, int offset, int direction) {
    return cg_align(type, offset, direction);
}
