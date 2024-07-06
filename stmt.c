//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static ASTnode *single_stmt();

static ASTnode *nop_stmt() {
    return make_ast_leaf(A_NOP, P_NONE, NULL, NULL, 0);
}

static ASTnode *if_stmt() {
    ASTnode *cond_node, *true_node, *false_node = NULL;

    scan();
    match(T_LPAREN, "(");
    cond_node = bin_expr(0);
    if (cond_node->op < A_EQ || cond_node->op > A_GE) {
        cond_node = make_ast_unary(A_TOBOOL, cond_node->type, cond_node->ctype, cond_node, NULL, 0);
    }
    match(T_RPAREN, ")");
    true_node = compound_stmt(FALSE);
    if (TOKEN.token_type == T_ELSE) {
        scan();
        false_node = compound_stmt(FALSE);
    }

    return make_ast_node(A_IF, P_NONE, NULL, cond_node, true_node, false_node, NULL, 0);
}

static ASTnode *while_stmt() {
    ASTnode *cond_node, *body_node;

    scan();
    match(T_LPAREN, "(");

    cond_node = bin_expr(0);
    if (cond_node->op < A_EQ || cond_node->op > A_GE) {
        cond_node = make_ast_unary(A_TOBOOL, cond_node->type, cond_node->ctype, cond_node, NULL, 0);
    }
    match(T_RPAREN, ")");

    LOOP_LEVEL++;
    body_node = compound_stmt(FALSE);
    LOOP_LEVEL--;

    return make_ast_node(A_WHILE, P_NONE, NULL, cond_node, NULL, body_node, NULL, 0);
}

static ASTnode *for_stmt() {
    ASTnode *cond_node, *body_node, *pre_node, *post_node, *tree;

    scan();
    match(T_LPAREN, "(");

    pre_node = expression_list(T_SEMI);
    scan();

    cond_node = bin_expr(0);
    if (cond_node->op < A_EQ || cond_node->op > A_GE) {
        cond_node = make_ast_unary(A_TOBOOL, cond_node->type, cond_node->ctype, cond_node, NULL, 0);
    }
    match(T_SEMI, ";");

    post_node = expression_list(T_RPAREN);
    scan();

    LOOP_LEVEL++;
    body_node = compound_stmt(FALSE);
    LOOP_LEVEL--;

    tree = make_ast_node(A_GLUE, P_NONE, NULL, body_node, NULL, post_node, NULL, 0);
    tree = make_ast_node(A_WHILE, P_NONE, NULL, cond_node, NULL, tree, NULL, 0);

    return make_ast_node(A_GLUE, P_NONE, NULL, pre_node, NULL, tree, NULL, 0);
}

static ASTnode *return_stmt() {
    ASTnode *tree = NULL;

    scan();

    if (FUNC_PTR->ptype == P_VOID) {
        if (TOKEN.token_type != T_SEMI) {
            fatal("Can't return from a void function");
        }
    } else {
        if (TOKEN.token_type == T_SEMI) {
            fatal("Must return a value from a non-void function");
        }
        tree = bin_expr(0);
        tree = modify_type(tree, make_ast_leaf(P_NONE, FUNC_PTR->ptype, FUNC_PTR->ctype, NULL, 0), P_NONE);
        if (!tree) {
            fatal("Incompatible return type");
        }
    }

    tree = make_ast_unary(A_RETURN, P_NONE, NULL, tree, NULL, 0);
    match(T_SEMI, ";");
    return tree;
}

static ASTnode *goto_stmt(int goto_type) {
    if (!(LOOP_LEVEL || SWITCH_LEVEL)) {
        fatal("No loop or switch to jump out of");
    }

    // skip the break/continue keyword
    scan();
    match(T_SEMI, ";");
    return make_ast_leaf(goto_type, P_NONE, NULL, NULL, 0);
}

static ASTnode *switch_stmt() {
    ASTnode *tree, *case_node, *left, *case_tree = NULL, *case_tail;
    int in_loop = TRUE, seen_dft = FALSE, case_cnt = 0, op, case_val;

    // skip the switch keyword
    scan();

    match(T_LPAREN, "(");
    left = bin_expr(0);
    match(T_RPAREN, ")");
    match(T_LBRACE, "{");
    if (!is_int(left->type)) {
        fatal("Switch expression must be an integer");
    }

    tree = make_ast_unary(A_SWITCH, P_NONE, NULL, left, NULL, 0);
    SWITCH_LEVEL++;

    while (in_loop) {
        switch (TOKEN.token_type) {
            case T_CASE:
            case T_DEFAULT:
                if (seen_dft) {
                    fatal("case or default after existing default");
                }
                if (TOKEN.token_type == T_DEFAULT) {
                    op = A_DEFAULT;
                    seen_dft = TRUE;
                    // skip the default keyword
                    scan();
                    case_val = 0;
                } else {
                    op = A_CASE;
                    // skip the case keyword
                    scan();
                    left = bin_expr(0);
                    if (left->op != A_INTLIT) {
                        fatal("Case expression must be an integer");
                    }
                    case_val = (int) left->int_value;

                    // check if there are duplicate case values
                    for (case_node = case_tree; case_node; case_node = case_node->right) {
                        if (case_val == (int) case_node->int_value) {
                            fatal("Duplicate case value");
                        }
                    }
                }
                match(T_COLON, ":");
                case_cnt++;
                switch (TOKEN.token_type) {
                    case T_CASE:
                    case T_DEFAULT:
                        left = nop_stmt();
                        break;
                    case T_LBRACE:
                        left = compound_stmt(FALSE);
                        break;
                    default:
                        left = compound_stmt(TRUE);
                        break;
                }

                if (case_tree) {
                    case_tail->right = make_ast_unary(op, P_NONE, NULL, left, NULL, case_val);
                    case_tail = case_tail->right;
                } else {
                    case_tree = case_tail = make_ast_unary(op, P_NONE, NULL, left, NULL, case_val);
                }

                break;
            case T_RBRACE:
                if (!case_cnt && !seen_dft) {
                    fatal("Switch statement has no cases or default");
                }
                in_loop = FALSE;
                break;
            default:
                fatals("Unexpected token in switch statement", get_name(V_TOKEN, TOKEN.token_type));
        }
    }

    SWITCH_LEVEL--;
    match(T_RBRACE, "}");

    tree->right = case_tree;
    tree->int_value = case_cnt;

    return tree;
}

static ASTnode *single_stmt() {
    Symbol *ctype;
    ASTnode *stmt = NULL;
    switch (TOKEN.token_type) {
        case T_SEMI:
            scan();
            break;
        case T_IDENT:
            if (!find_typedef_sym(TEXT)) {
                stmt = bin_expr(0);
                break;
            }
        case T_CHAR:
        case T_SHORT:
        case T_INT:
        case T_LONG:
        case T_STRUCT:
        case T_UNION:
        case T_ENUM:
        case T_TYPEDEF:
            declare_list(&ctype, C_LOCAL, T_SEMI, T_EOF, &stmt, NULL, NULL);
            if (stmt && stmt->op == A_ASSIGN) { // A_ASSIGN will be handled behind
            } else {
                match(T_SEMI, ";");
            }
            break;
        case T_IF:
            stmt = if_stmt();
            break;
        case T_WHILE:
            stmt = while_stmt();
            break;
        case T_FOR:
            stmt = for_stmt();
            break;
        case T_RETURN:
            stmt = return_stmt();
            break;
        case T_BREAK:
            stmt = goto_stmt(A_BREAK);
            break;
        case T_CONTINUE:
            stmt = goto_stmt(A_CONTINUE);
            break;
        case T_SWITCH:
            stmt = switch_stmt();
            break;
        default:
            stmt = bin_expr(0);
            break;
    }

    if (stmt) {
        stmt->line = LINE;
    }

    return stmt;
}

/*
 * is_switch:
 * - TRUE if this is a case statement from switch
 * - which starts without a left brace,
 * - otherwise, case statement should call this
 * - function with is_switch == FALSE
 */
ASTnode *compound_stmt(int is_switch) {
    ASTnode *tree, *left = NULL;
    int single_mode = FALSE;

    if (!is_switch) {
        // single statement mode
        if (TOKEN.token_type != T_LBRACE) {
            single_mode = TRUE;
        } else {
            // otherwise, must start with a left brace
            match(T_LBRACE, "{");
        }
    }

    while (TRUE) {
        if (is_switch &&
            (TOKEN.token_type == T_CASE || TOKEN.token_type == T_DEFAULT || TOKEN.token_type == T_RBRACE)) {
            return left;
        }
        if (TOKEN.token_type == T_RBRACE) {
            scan();
            return left;
        }

        tree = single_stmt();

        if (tree) {
            switch (tree->op) {
                case A_ASPLUS:
                case A_ASMINUS:
                case A_ASSTAR:
                case A_ASSLASH:
                case A_ASSIGN:
                case A_PREINC:
                case A_PREDEC:
                case A_POSTINC:
                case A_POSTDEC:
                case A_FUNCCALL:
                    match(T_SEMI, ";");
                default:
                    break;
            }
        }

        if (single_mode) {
            return tree;
        }

        if (tree) {
            if (left) {
                left = make_ast_node(A_GLUE, P_NONE, NULL, left, NULL, tree, NULL, 0);
            } else {
                left = tree;
            }
        }
    }
}
