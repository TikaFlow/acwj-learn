//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static int parse_type(int t) {
    int type = 0;
    switch (t) {
        case T_CHAR:
            type = P_CHAR;
            break;
        case T_INT:
            type = P_INT;
            break;
        case T_VOID:
            type = P_VOID;
            break;
        default:
            fatald("Illegal type, token", t);
    }
    scan(&TOKEN);
    return type;
}

void declare_var() {
    int type = parse_type(TOKEN.token);
    match(T_IDENT, "identifier");
    int id = add_sym(TEXT, type, S_VARIABLE);
    gen_new_sym(id);
    match(T_SEMI, ";");
}

ASTnode *declare_func() {
    ASTnode *tree;
    int nameslot;

    match(T_VOID, "void");
    match(T_IDENT, "identifier");
    nameslot = add_sym(TEXT, P_VOID, S_FUNCTION);
    match(T_LPAREN, "(");
    match(T_RPAREN, ")");

    tree = compound_stmt();

    return make_ast_unary(A_FUNCTION, P_VOID, tree, nameslot);
}
