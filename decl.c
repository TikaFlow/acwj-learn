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

void declare_var(int type, int is_local, int is_param) {
    if (TOKEN.token_type == T_LBRACKET) {
        match(T_LBRACKET, "[");

        if (TOKEN.token_type == T_INTLIT) {
            if (is_local) {
                // add_local_sym(TEXT, pointer_to(type), S_ARRAY, 0, (int) TOKEN.int_value);
                fatal("For now, declaration of local arrays is not implemented");
            } else {
                add_global_sym(TEXT, pointer_to(type), S_ARRAY, 0, (int) TOKEN.int_value);
            }
        }

        scan();
        match(T_RBRACKET, "]");
    } else {
        if (is_local) {
            if (add_local_sym(TEXT, type, S_VARIABLE, is_param, 1) == -1) {
                fatals("Local variable redeclaration", TEXT);
            }
        } else {
            add_global_sym(TEXT, type, S_VARIABLE, 0, 1);
        }
    }
}

static int declare_params() {
    int type, param_cnt = 0;

    while (TOKEN.token_type != T_RPAREN) {
        type = parse_type();
        match(T_IDENT, "identifier");
        declare_var(type, TRUE, TRUE);
        param_cnt++;

        switch (TOKEN.token_type) {
            case T_COMMA:
                match(T_COMMA, ",");
            case T_RPAREN:
                break;
            default:
                fatald("Unexpected token in parameter list", TOKEN.token_type);
        }
    }

    return param_cnt;
}

ASTnode *declare_func(int type) {
    ASTnode *tree, *final_stmt;
    int slot, end_label = gen_label();
    FUNC_ID = slot = add_global_sym(TEXT, type, S_FUNCTION, end_label, 0);
    match(T_LPAREN, "(");
    SYM_TAB[FUNC_ID].posn = declare_params();
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

    return make_ast_unary(A_FUNCTION, type, tree, slot);
}

void multi_declare_var(int type, int is_local, int is_param) {
    while (TRUE) {
        declare_var(type, is_local, is_param);

        if (TOKEN.token_type == T_SEMI) {
            match(T_SEMI, ";");
            break;
        }

        match(T_COMMA, ",");
        match(T_IDENT, "identifier");
    }
}

void declare_global() {
    ASTnode *tree;
    int type;

    while (TRUE) {
        type = parse_type();
        match(T_IDENT, "identifier");

        if (TOKEN.token_type == T_LPAREN) {
            tree = declare_func(type);
            // show_ast(tree, NO_LABEL, 0); // SHOW_AST
            // fprintf(stdout, "\n\n");
            gen_ast(tree, NO_LABEL, 0);
            reset_loccal_syms();
        } else {
            multi_declare_var(type, FALSE, FALSE);
        }

        if (TOKEN.token_type == T_EOF) {
            break;
        }
    }
}
