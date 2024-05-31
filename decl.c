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

void declare_var(int type, int class) {
    if (TOKEN.token_type == T_LBRACKET) {
        match(T_LBRACKET, "[");

        if (TOKEN.token_type == T_INTLIT) {
            if (class == C_LOCAL) {
                // add_local_sym(TEXT, pointer_to(type), S_ARRAY, 0, (int) TOKEN.int_value);
                fatal("For now, declaration of local arrays is not implemented");
            } else {
                add_global_sym(TEXT, pointer_to(type), S_ARRAY, class, (int) TOKEN.int_value);
            }
        }

        scan();
        match(T_RBRACKET, "]");
    } else {
        if (class == C_LOCAL) {
            if (add_local_sym(TEXT, type, S_VARIABLE, class, 1) == -1) {
                fatals("Local variable redeclaration", TEXT);
            }
        } else {
            add_global_sym(TEXT, type, S_VARIABLE, class, 1);
        }
    }
}

static int declare_params(int id) {
    int type, origin_param_cnt, param_id = id + 1, param_cnt = 0;

    if (param_id) {
        origin_param_cnt = SYM_TAB[id].n_param;
    }

    while (TOKEN.token_type != T_RPAREN) {
        type = parse_type();
        match(T_IDENT, "identifier");

        if (param_id) {
            if (type != SYM_TAB[id].ptype) {
                fatald("Type mismatch of parameter", param_cnt);
            }
            param_id++;
        } else {
            declare_var(type, C_PARAM);
        }

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

    if (id != NOT_FOUND && param_cnt != origin_param_cnt) {
        fatals("Parameter count mismatch for function", SYM_TAB[id].name);
    }

    return param_cnt;
}

ASTnode *declare_func(int type) {
    ASTnode *tree, *final_stmt;
    int id, slot, end_label, param_cnt;

    if ((id = find_sym(TEXT)) != NOT_FOUND && SYM_TAB[id].stype != S_FUNCTION) {
        id = NOT_FOUND;
    }

    if (id == NOT_FOUND) {
        end_label = gen_label();
        slot = add_global_sym(TEXT, type, S_FUNCTION, C_GLOBAL, end_label);
    }

    match(T_LPAREN, "(");
    param_cnt = declare_params(id);
    match(T_RPAREN, ")");

    if (id == NOT_FOUND) {
        SYM_TAB[slot].n_param = param_cnt;
    }

    if (TOKEN.token_type == T_SEMI) {
        match(T_SEMI, ";");
        return NULL;
    }

    if (id == NOT_FOUND) {
        id = slot;
    }
    copy_func_params(id);
    FUNC_ID = id;

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

    return make_ast_unary(A_FUNCTION, type, tree, id);
}

void multi_declare_var(int type, int class) {
    while (TRUE) {
        declare_var(type, class);

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
            if (!tree) {
                continue; // for prototype, no code to generate
            }

            if (FLAG_T) {
                dump_ast(tree, NO_LABEL, 0); // SHOW_AST
                fprintf(stdout, "\n\n");
            }

            gen_ast(tree, NO_LABEL, 0);
            reset_loccal_syms();
        } else {
            multi_declare_var(type, C_GLOBAL);
        }

        if (TOKEN.token_type == T_EOF) {
            break;
        }
    }
}
