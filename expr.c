//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

static int op_prec[] = {
        0, 10, 20, 30,                // T_EOF, T_ASSIGN, T_LOGOR, T_LOGAND
        40, 50, 60,                   // T_OR, T_XOR, T_AND
        70, 70,                       // T_EQ, T_NE
        80, 80, 80, 80,               // T_LT, T_GT, T_LE, T_GE
        90, 90,                       // T_LSHIFT, T_RSHIFT
        100, 100,                     // T_PLUS, T_MINUS
        110, 110                      // T_STAR, T_SLASH
};

static ASTnode *param_list() {
    ASTnode *node = NULL, *param_node = NULL;
    int param_cnt = 0;

    while (TOKEN.token_type != T_RPAREN) {
        param_node = bin_expr(0);
        node = make_ast_node(A_GLUE, P_NONE, node, NULL, param_node, param_cnt++);

        switch (TOKEN.token_type) {
            case T_COMMA:
                match(T_COMMA, ",");
            case T_RPAREN:
                break;
            default:
                fatald("Unexpected token in param list", TOKEN.token_type);
        }
    }

    return node;
}

ASTnode *func_call() {
    int id;

    if ((id = find_sym(TEXT)) == NOT_FOUND || SYM_TAB[id].stype != S_FUNCTION) {
        fatals("Undeclared function", TEXT);
    }
    match(T_LPAREN, "(");
    ASTnode *node = param_list();
    // TODO check params against prototype
    node = make_ast_unary(A_FUNCCALL, SYM_TAB[id].ptype, node, id);
    match(T_RPAREN, ")");
    return node;
}

static ASTnode *access_array() {
    ASTnode *left, *right;
    int id;

    if ((id = find_sym(TEXT)) == NOT_FOUND || SYM_TAB[id].stype != S_ARRAY) {
        fatals("Undeclared array", TEXT);
    }

    left = make_ast_leaf(A_ADDR, SYM_TAB[id].ptype, id);
    match(T_LBRACKET, "[");
    right = bin_expr(0);
    match(T_RBRACKET, "]");

    if (!is_int(right->type)) {
        fatal("Array index must be an integer");
    }

    right = modify_type(right, left->type, A_ADD);
    left = make_ast_node(A_ADD, SYM_TAB[id].ptype, left, NULL, right, 0);
    left = make_ast_unary(A_DEREF, value_at(left->type), left, 0);

    return left;
}

static ASTnode *postfix() {
    ASTnode *node;
    int id;

    scan();
    if (TOKEN.token_type == T_LPAREN) {
        return func_call();
    }
    if (TOKEN.token_type == T_LBRACKET) {
        return access_array();
    }
    // reject_token();

    if ((id = find_sym(TEXT)) == NOT_FOUND || SYM_TAB[id].stype != S_VARIABLE) {
        fatals("Unknown variable", TEXT);
    }

    switch (TOKEN.token_type) {
        case T_INC:
            match(T_INC, "++");
            node = make_ast_leaf(A_POSTINC, SYM_TAB[id].ptype, id);
            break;
        case T_DEC:
            match(T_DEC, "--");
            node = make_ast_leaf(A_POSTDEC, SYM_TAB[id].ptype, id);
            break;
        default:
            node = make_ast_leaf(A_IDENT, SYM_TAB[id].ptype, id);
    }

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
        case T_STRLIT:
            id = gen_new_str(TEXT);
            node = make_ast_leaf(A_STRLIT, P_CHARPTR, id);
            break;
        case T_IDENT:
            return postfix();
        case T_LPAREN:
            match(T_LPAREN, "(");
            node = bin_expr(0);
            match(T_RPAREN, ")");
            return node;
        default:
            node = NULL;
            fatald("syntax error, token", TOKEN.token_type);
    }

    scan();
    return node;
}

static int token_to_op(int tk) {
    if (tk > T_EOF && tk < T_SLASH) {
        return tk;
    }
    fatald("Unknown token", TOKEN.token_type);
    return 0;
}

static int stick_right(int token_type) {
    if (token_type == T_ASSIGN) {
        return TRUE;
    }
    return FALSE;
}

static int op_precedence(int token_type) {
    if (token_type == T_VOID) {
        fatald("Token with no precendence", token_type);
    }

    int prec = op_prec[token_type];
    if (prec == 0) {
        fatald("Syntax error, token type", token_type);
    }
    return prec;
}

static ASTnode *prefix() {
    ASTnode *tree;

    switch (TOKEN.token_type) {
        case T_AND:
            match(T_AND, "&");
            tree = prefix();

            if (tree->op != A_IDENT) {
                fatal("& operator must be followed by a variable");
            }

            tree->op = A_ADDR;
            tree->type = pointer_to(tree->type);
            break;
        case T_STAR:
            match(T_STAR, "*");
            tree = prefix();

            if (tree->op != A_IDENT && tree->op != A_DEREF) {
                fatal("* operator must be followed by a variable or *");
            }

            tree = make_ast_unary(A_DEREF, value_at(tree->type), tree, 0);
            break;
        case T_MINUS:
            match(T_MINUS, "-");
            tree = prefix();

            tree->rvalue = 1;
            tree = modify_type(tree, P_INT, 0);
            tree = make_ast_unary(A_NEGATE, tree->type, tree, 0);
            break;
        case T_INVERT:
            match(T_INVERT, "~");
            tree = prefix();

            tree->rvalue = 1;
            tree = make_ast_unary(A_INVERT, tree->type, tree, 0);
            break;
        case T_LOGNOT:
            match(T_LOGNOT, "!");
            tree = prefix();

            tree->rvalue = 1;
            tree = make_ast_unary(A_LOGNOT, tree->type, tree, 0);
            break;
        case T_INC:
            match(T_INC, "++");
            tree = prefix();

            if (tree->op != A_IDENT) {
                fatal("++ operator must be followed by a variable");
            }
            tree = make_ast_unary(A_PREINC, tree->type, tree, 0);
            break;
        case T_DEC:
            match(T_DEC, "--");
            tree = prefix();

            if (tree->op != A_IDENT) {
                fatal("++ operator must be followed by a variable");
            }
            tree = make_ast_unary(A_PREDEC, tree->type, tree, 0);
            break;
        case T_PLUS:
            match(T_PLUS, "+");
        default:
            tree = primary();
    }

    return tree;
}

/*
 * @param ptp: previous token precedence
 */
ASTnode *bin_expr(int ptp) {
    ASTnode *ltemp, *rtemp, *right, *left = prefix();
    int ast_op, token_type = TOKEN.token_type;

    switch (token_type) {
        case T_SEMI:
        case T_RPAREN:
        case T_RBRACKET:
        case T_COMMA:
            left->rvalue = 1;
            return left;
        default:
            break;
    }

    while (op_precedence(token_type) > ptp
           || (stick_right(token_type) && op_precedence(token_type) == ptp)) {
        scan();

        right = bin_expr(op_prec[token_type]);

        ast_op = token_to_op(token_type);
        if (ast_op == A_ASSIGN) {
            right->rvalue = 1;

            right = modify_type(right, left->type, 0);
            if (!right) {
                fatal("Incompatible expression in assignment");
            }
            // swap left and right
            ltemp = left;
            left = right;
            right = ltemp;
        } else {
            left->rvalue = 1;
            right->rvalue = 1;

            ltemp = modify_type(left, right->type, ast_op);
            rtemp = modify_type(right, left->type, ast_op);
            if (!ltemp && !rtemp) {
                fatal("Incompatible types in arithmetic expression");
            }

            if (ltemp) {
                left = ltemp;
            }
            if (rtemp) {
                right = rtemp;
            }
        }

        left = make_ast_node(token_to_op(token_type), left->type, left, NULL, right, 0);

        token_type = TOKEN.token_type;
        switch (token_type) {
            case T_SEMI:
            case T_RPAREN:
            case T_RBRACKET:
            case T_COMMA:
                left->rvalue = 1;
                return left;
            default:
                break;
        }
    }

    left->rvalue = 1;
    return left;
}
