//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static int parse_type(int t) {
    int type = 0;
    switch (t) {
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
            fatald("Illegal type, token", t);
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

void declare_var() {
    int type = parse_type(TOKEN.token_type);
    match(T_IDENT, "identifier");
    int id = add_sym(TEXT, type, S_VARIABLE, 0);
    gen_new_sym(id);
    match(T_SEMI, ";");
}

ASTnode *declare_func() {
    ASTnode *tree, *final_stmt;
    int nameslot, end_label, type = parse_type(TOKEN.token_type);

    match(T_IDENT, "identifier");
    end_label = gen_label();
    FUNC_ID = nameslot = add_sym(TEXT, type, S_FUNCTION, end_label);
    match(T_LPAREN, "(");
    match(T_RPAREN, ")");

    tree = compound_stmt();

    if (type != P_VOID) {
        final_stmt = tree->op == A_GLUE ? tree->right : tree;
        if (final_stmt == NULL || final_stmt->op != A_RETURN) {
            fatal("No return statement in function with non-void return type");
        }
    }

    return make_ast_unary(A_FUNCTION, P_VOID, tree, nameslot);
}
