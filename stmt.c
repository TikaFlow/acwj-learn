//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static ASTnode *single_stmt();

static ASTnode *print_stmt() {
    match(T_PRINT, "print");

    ASTnode *tree = bin_expr(0);

    tree = modify_type(tree, P_INT, 0);
    if (!tree) {
        fatal("Incompatible types in print statement");
    }

    tree = make_ast_unary(A_PRINT, P_NONE, tree, 0);

    return tree;
}

static ASTnode *assign_stmt() {
    ASTnode *left, *right, *tree;
    int id;

    match(T_IDENT, "identifier");

    if (TOKEN.token_type == T_LPAREN) {
        return func_call();
    }

    if ((id = find_sym(TEXT)) < 0) {
        fatals("undeclared variable", TEXT);
    }
    right = make_ast_leaf(A_LVIDENT, SYM_TAB[id].ptype, id);

    match(T_ASSIGN, "=");
    left = bin_expr(0);

    left = modify_type(left, right->type, 0);
    if (!left) {
        fatal("Incompatible types in assignment");
    }

    tree = make_ast_node(A_ASSIGN, P_INT, left, NULL, right, 0);

    return tree;
}

static ASTnode *if_stmt() {
    ASTnode *condnode, *truenode, *falsenode = NULL;

    match(T_IF, "if");
    match(T_LPAREN, "(");
    condnode = bin_expr(0);
    if (condnode->op < A_EQ || condnode->op > A_GE) {
        fatal("Bad comparison operator");
    }
    match(T_RPAREN, ")");
    truenode = compound_stmt();
    if (TOKEN.token_type == T_ELSE) {
        match(T_ELSE, "else");
        falsenode = compound_stmt();
    }

    return make_ast_node(A_IF, P_NONE, condnode, truenode, falsenode, 0);
}

static ASTnode *while_stmt() {
    ASTnode *condnode, *bodenode;

    match(T_WHILE, "while");
    match(T_LPAREN, "(");

    condnode = bin_expr(0);
    if (condnode->op < A_EQ || condnode->op > A_GE) {
        fatal("Bad comparison operator");
    }
    match(T_RPAREN, ")");

    bodenode = compound_stmt();

    return make_ast_node(A_WHILE, P_NONE, condnode, NULL, bodenode, 0);
}

static ASTnode *for_stmt() {
    ASTnode *condnode, *bodenode, *preopnode, *postopnode, *tree;

    match(T_FOR, "for");
    match(T_LPAREN, "(");

    preopnode = single_stmt();
    match(T_SEMI, ";");

    condnode = bin_expr(0);
    if (condnode->op < A_EQ || condnode->op > A_GE) {
        fatal("Bad comparison operator");
    }
    match(T_SEMI, ";");

    postopnode = single_stmt();
    match(T_RPAREN, ")");

    bodenode = compound_stmt();

    tree = make_ast_node(A_GLUE, P_NONE, bodenode, NULL, postopnode, 0);
    tree = make_ast_node(A_WHILE, P_NONE, condnode, NULL, tree, 0);

    return make_ast_node(A_GLUE, P_NONE, preopnode, NULL, tree, 0);
}

static ASTnode *return_stmt() {
    ASTnode *tree;
    int return_type, func_type;
    int need_rparen = FALSE;

    if (SYM_TAB[FUNC_ID].ptype == P_VOID) {
        fatal("Can't return from a void function");
    }

    match(T_RETURN, "return");

    if (TOKEN.token_type == T_LPAREN) {
        match(T_LPAREN, "(");
        need_rparen = TRUE;
    }

    tree = bin_expr(0);
    tree = modify_type(tree, SYM_TAB[FUNC_ID].ptype, 0);
    if (!tree) {
        fatal("Incompatible return type");
    }

    tree = make_ast_unary(A_RETURN, P_NONE, tree, 0);
    if (need_rparen) {
        match(T_RPAREN, ")");
    }
    return tree;
}

static ASTnode *single_stmt() {
    int type;
    switch (TOKEN.token_type) {
        case T_PRINT:
            return print_stmt();
        case T_CHAR:
        case T_INT:
        case T_LONG:
            type = parse_type();
            match(T_IDENT, "identifier");
            declare_var(type);
            return NULL;
        case T_IDENT:
            return assign_stmt();
        case T_IF:
            return if_stmt();
        case T_WHILE:
            return while_stmt();
        case T_FOR:
            return for_stmt();
        case T_RETURN:
            return return_stmt();
        default:
            fatald("syntax error, unexpected token", TOKEN.token_type);
    }
    return NULL;
}

ASTnode *compound_stmt() {
    ASTnode *tree, *left = NULL;
    match(T_LBRACE, "{");

    while (1) {
        tree = single_stmt();

        if (tree &&
            (tree->op == A_PRINT
             || tree->op == A_ASSIGN
             || tree->op == A_RETURN
             || tree->op == A_FUNCCALL)) {
            match(T_SEMI, ";");
        }

        if (tree) {
            if (left) {
                left = make_ast_node(A_GLUE, P_NONE, left, NULL, tree, 0);
            } else {
                left = tree;
            }
        }

        if (TOKEN.token_type == T_RBRACE) {
            match(T_RBRACE, "}");
            return left;
        }
    }
}
