//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static ASTnode *single_stmt();

static ASTnode *if_stmt() {
    ASTnode *cond_node, *true_node, *false_node = NULL;

    match(T_IF, "if");
    match(T_LPAREN, "(");
    cond_node = bin_expr(0);
    if (cond_node->op < A_EQ || cond_node->op > A_GE) {
        cond_node = make_ast_unary(A_TOBOOL, cond_node->type, cond_node, NULL, 0);
    }
    match(T_RPAREN, ")");
    true_node = compound_stmt();
    if (TOKEN.token_type == T_ELSE) {
        match(T_ELSE, "else");
        false_node = compound_stmt();
    }

    return make_ast_node(A_IF, P_NONE, cond_node, true_node, false_node, NULL, 0);
}

static ASTnode *while_stmt() {
    ASTnode *cond_node, *body_node;

    match(T_WHILE, "while");
    match(T_LPAREN, "(");

    cond_node = bin_expr(0);
    if (cond_node->op < A_EQ || cond_node->op > A_GE) {
        cond_node = make_ast_unary(A_TOBOOL, cond_node->type, cond_node, NULL, 0);
    }
    match(T_RPAREN, ")");

    LOOP_LEVEL++;
    body_node = compound_stmt();
    LOOP_LEVEL--;

    return make_ast_node(A_WHILE, P_NONE, cond_node, NULL, body_node, NULL, 0);
}

static ASTnode *for_stmt() {
    ASTnode *cond_node, *body_node, *pre_node, *post_node, *tree;

    match(T_FOR, "for");
    match(T_LPAREN, "(");

    pre_node = single_stmt();
    match(T_SEMI, ";");

    cond_node = bin_expr(0);
    if (cond_node->op < A_EQ || cond_node->op > A_GE) {
        cond_node = make_ast_unary(A_TOBOOL, cond_node->type, cond_node, NULL, 0);
    }
    match(T_SEMI, ";");

    post_node = single_stmt();
    match(T_RPAREN, ")");

    LOOP_LEVEL++;
    body_node = compound_stmt();
    LOOP_LEVEL--;

    tree = make_ast_node(A_GLUE, P_NONE, body_node, NULL, post_node, NULL, 0);
    tree = make_ast_node(A_WHILE, P_NONE, cond_node, NULL, tree, NULL, 0);

    return make_ast_node(A_GLUE, P_NONE, pre_node, NULL, tree, NULL, 0);
}

static ASTnode *return_stmt() {
    ASTnode *tree;

    if (FUNC_PTR->ptype == P_VOID) {
        fatal("Can't return from a void function");
    }

    match(T_RETURN, "return");

    tree = bin_expr(0);
    tree = modify_type(tree, FUNC_PTR->ptype, P_NONE);
    if (!tree) {
        fatal("Incompatible return type");
    }

    tree = make_ast_unary(A_RETURN, P_NONE, tree, NULL, 0);
    return tree;
}

static ASTnode *goto_stmt(int goto_type) {
    if (!LOOP_LEVEL) {
        fatal("No loop to jump out of");
    }

    // skip the break keyword
    scan();
    return make_ast_leaf(goto_type, P_NONE, NULL, 0);
}

static ASTnode *single_stmt() {
    Symbol *ctype;
    int type, class = C_LOCAL;
    switch (TOKEN.token_type) {
        case T_IDENT:
            if (!find_typedef_sym(TEXT)) {
                return bin_expr(0);
            }
        case T_CHAR:
        case T_INT:
        case T_LONG:
        case T_STRUCT:
        case T_UNION:
        case T_ENUM:
        case T_TYPEDEF:
            type = parse_type(&ctype, &class);

            if (type == P_NONE) {
                match(T_SEMI, ";");
                return NULL;
            }

            match(T_IDENT, "identifier");
            multi_declare_var(type, ctype, class);
            return NULL;
        case T_IF:
            return if_stmt();
        case T_WHILE:
            return while_stmt();
        case T_FOR:
            return for_stmt();
        case T_RETURN:
            return return_stmt();
        case T_BREAK:
            return goto_stmt(A_BREAK);
        case T_CONTINUE:
            return goto_stmt(A_CONTINUE);
        default:
            return bin_expr(0);
    }
}

ASTnode *compound_stmt() {
    ASTnode *tree, *left = NULL;
    match(T_LBRACE, "{");

    while (TRUE) {
        tree = single_stmt();

        if (tree) {
            switch (tree->op) {
                case A_ASSIGN:
                case A_RETURN:
                case A_FUNCCALL:
                case A_BREAK:
                case A_CONTINUE:
                    match(T_SEMI, ";");
                default:
                    break;
            }
        }

        if (tree) {
            if (left) {
                left = make_ast_node(A_GLUE, P_NONE, left, NULL, tree, NULL, 0);
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
