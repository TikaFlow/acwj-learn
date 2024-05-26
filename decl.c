//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

int parse_type() {
    int type = 0;
    switch (TOKEN.token_type) {
        case T_VOID:
            type = P_VOID;
            break;
        case T_CHAR:
            type = P_CHAR;
            break;
        case T_INT:
            type = P_INT;
            break;
        case T_LONG:
            type = P_LONG;
            break;
        default:
            fatald("Illegal type, token", TOKEN.token_type);
    }

    while (TRUE) {
        scan();
        if (TOKEN.token_type != T_STAR) {
            break;
        }
        type = pointer_to(type);
    }

    return type;
}

void declare_var(int type) {
    int id;

    while (TRUE) {
        id = add_sym(TEXT, type, S_VARIABLE, 0);
        gen_new_sym(id);

        if (TOKEN.token_type == T_SEMI) {
            match(T_SEMI, ";");
            return;
        }

        match(T_COMMA, ",");
        match(T_IDENT, "identifier");
    }
}

ASTnode *declare_func(int type) {
    ASTnode *tree, *final_stmt;
    int nameslot, end_label = gen_label();
    FUNC_ID = nameslot = add_sym(TEXT, type, S_FUNCTION, end_label);
    match(T_LPAREN, "(");
    // for now, no parameters
    match(T_RPAREN, ")");

    tree = compound_stmt();

    if (type != P_VOID) {
        if (!tree) {
            fatal("No statements in function with non-void type");
        }

        final_stmt = tree->op == A_GLUE ? tree->right : tree;
        if (!final_stmt || final_stmt->op != A_RETURN) {
            fatal("No return statement in function with non-void return type");
        }
    }

    return make_ast_unary(A_FUNCTION, type, tree, nameslot);
}

void declare_global() {
    ASTnode *tree;
    int type;

    while (TRUE) {
        type = parse_type();
        match(T_IDENT, "identifier");

        if (TOKEN.token_type == T_LPAREN) {
            tree = declare_func(type);
            show_ast(tree, NO_LABEL, 0); // SHOW_AST
            fprintf(stdout, "\n\n");
            gen_ast(tree, NO_LABEL, 0);
        } else {
            declare_var(type);
        }

        if (TOKEN.token_type == T_EOF) {
            break;
        }
    }
}
