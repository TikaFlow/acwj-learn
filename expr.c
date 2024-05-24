//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

ASTnode *func_call() {
    int id;

    if ((id = find_sym(TEXT)) < 0) {
        fatals("Undeclared function", TEXT);
    }
    match(T_LPAREN, "(");
    ASTnode *node = bin_expr(0);
    node = make_ast_unary(A_FUNCCALL, SYM_TAB[id].ptype, node, id);
    match(T_RPAREN, ")");
    return node;
}

static ASTnode *primary() {
    ASTnode *node;
    int id;

    switch (TOKEN.token_type) {
        case T_INTLIT:
            if (TOKEN.int_value >= 0 && TOKEN.int_value <= 255) {
                node = make_ast_leaf(A_INTLIT, P_CHAR, TOKEN.int_value);
            } else {
                node = make_ast_leaf(A_INTLIT, P_INT, TOKEN.int_value);
            }
            break;
        case T_IDENT:
            scan();
            if (TOKEN.token_type == T_LPAREN) {
                return func_call();
            }
            peek_token();

            if ((id = find_sym(TEXT)) < 0) {
                fatals("Unknown variable", TEXT);
            }
            node = make_ast_leaf(A_IDENT, SYM_TAB[id].ptype, id);
            break;
        default:
            node = NULL;
            fatald("syntax error, token", TOKEN.token_type);
    }

    scan();
    return node;
}

static int token_to_op(int tk) {
    if (tk > T_EOF && tk < T_INTLIT) {
        return tk;
    }
    fatald("Unknown token", TOKEN.token_type);
    return 0;
}

static int op_prec[] = {
        0, 10, 10, // EOF + -
        20, 20, // * /
        30, 30, // == !=
        40, 40, 40, 40 // < <= > >=
};

static int op_precedence(int token_type) {
    int prec = op_prec[token_type];
    if (prec == 0) {
        fatald("Syntax error, token type", token_type);
    }
    return prec;
}

/*
 * @param ptp: previous token precedence
 */
ASTnode *bin_expr(int ptp) {
    ASTnode *right, *left = primary();
    int left_type, right_type;
    int token_type = TOKEN.token_type;

    if (token_type == T_SEMI || token_type == T_RPAREN) {
        return left;
    }

    while (op_precedence(token_type) > ptp) {
        scan();

        right = bin_expr(op_prec[token_type]);

        left_type = left->type;
        right_type = right->type;
        if (!type_compatible(&left_type, &right_type, FALSE)) {
            fatal("Incompatible types in arithmetic expression");
        }
        if (left_type) {
            left = make_ast_unary(left_type, right->type, left, 0);
        }
        if (right_type) {
            right = make_ast_unary(right_type, left->type, right, 0);
        }

        left = make_ast_node(token_to_op(token_type), left->type, left, NULL, right, 0);

        token_type = TOKEN.token_type;
        if (token_type == T_SEMI || token_type == T_RPAREN) {
            return left;
        }
    }

    return left;
}
