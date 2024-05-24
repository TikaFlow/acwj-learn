//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

static ASTnode *primary() {
    ASTnode *node;
    int id;

    switch (TOKEN.token) {
        case T_INTLIT:
            if (TOKEN.intvalue >= 0 && TOKEN.intvalue <= 255) {
                node = make_ast_leaf(A_INTLIT, P_CHAR, TOKEN.intvalue);
            } else {
                node = make_ast_leaf(A_INTLIT, P_INT, TOKEN.intvalue);
            }
            break;
        case T_IDENT:
            if ((id = find_sym(TEXT)) < 0) {
                fatals("Unknown variable", TEXT);
            }
            node = make_ast_leaf(A_IDENT, SYM_TAB[id].ptype, id);
            break;
        default:
            node = NULL;
            fatald("syntax error, token", TOKEN.token);
    }

    scan(&TOKEN);
    return node;
}

static int token_to_op(int tk) {
    if (tk > T_EOF && tk < T_INTLIT) {
        return tk;
    }
    fatald("Unknown token", TOKEN.token);
    return 0;
}

static int op_prec[] = {
        0, 10, 10, // EOF + -
        20, 20, // * /
        30, 30, // == !=
        40, 40, 40, 40 // < <= > >=
};

static int op_precedence(int tokentype) {
    int prec = op_prec[tokentype];
    if (prec == 0) {
        fatald("Syntax error, token type", tokentype);
    }
    return prec;
}

/*
 * @param ptp: previous token precedence
 */
ASTnode *bin_expr(int ptp) {
    ASTnode *right, *left = primary();
    int left_type, right_type;
    int tokentype = TOKEN.token;

    if (tokentype == T_SEMI || tokentype == T_RPAREN) {
        return left;
    }

    while (op_precedence(tokentype) > ptp) {
        scan(&TOKEN);

        right = bin_expr(op_prec[tokentype]);

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

        left = make_ast_node(token_to_op(tokentype), left->type, left, NULL, right, 0);

        tokentype = TOKEN.token;
        if (tokentype == T_SEMI || tokentype == T_RPAREN) {
            return left;
        }
    }

    return left;
}
