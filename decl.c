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

Symbol *declare_var(int type, int class) {
    Symbol *sym = NULL;

    switch (class) {
        case C_GLOBAL:
            if (find_global(TEXT)) {
                fatals("Global variable redeclaration", TEXT);
            }
        case C_LOCAL:
        case C_PARAM:
            if (find_local(TEXT)) {
                fatals("Local variable redeclaration", TEXT);
            }
        default:
            break;
    }

    if (TOKEN.token_type == T_LBRACKET) {
        match(T_LBRACKET, "[");

        if (TOKEN.token_type == T_INTLIT) {
            switch (class) {
                case C_GLOBAL:
                    sym = add_global_sym(TEXT, pointer_to(type), S_ARRAY, class, (int) TOKEN.int_value);
                    break;
                case C_LOCAL:
                case C_PARAM:
                    fatal("For now, declaration of local arrays is not implemented");
                default:
                    break;
            }
        }

        scan();
        match(T_RBRACKET, "]");
    } else {
        switch (class) {
            case C_GLOBAL:
                sym = add_global_sym(TEXT, type, S_VARIABLE, class, 1);
                break;
            case C_LOCAL:
                sym = add_local_sym(TEXT, type, S_VARIABLE, class, 1);
                break;
            case C_PARAM:
                sym = add_param_sym(TEXT, type, S_VARIABLE, class, 1);
                break;
            default:
                break;
        }
    }

    return sym;
}

static int declare_params(Symbol *func_sym) {
    int type, param_cnt = 0;
    Symbol *proto_ptr = NULL;

    if (func_sym) {
        proto_ptr = func_sym->first;
    }

    while (TOKEN.token_type != T_RPAREN) {
        type = parse_type();
        match(T_IDENT, "identifier");

        if (proto_ptr) {
            if (type != proto_ptr->ptype) {
                fatald("Type mismatch of parameter", param_cnt);
            }
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

    if (func_sym && param_cnt != func_sym->n_param) {
        fatals("Parameter count mismatch for function", func_sym->name);
    }

    return param_cnt;
}

ASTnode *declare_func(int type) {
    ASTnode *tree, *final_stmt;
    Symbol *old_func, *new_func = NULL;
    int end_label, param_cnt;

    if ((old_func = find_global(TEXT)) && old_func->stype != S_FUNCTION) {
        old_func = NULL;
    }

    if (!old_func) {
        end_label = gen_label();
        new_func = add_global_sym(TEXT, type, S_FUNCTION, C_GLOBAL, end_label);
    }

    match(T_LPAREN, "(");
    param_cnt = declare_params(old_func);
    match(T_RPAREN, ")");

    if (new_func) {
        new_func->n_param = param_cnt;
        new_func->first = PARAM_HEAD;
        old_func = new_func;
    }
    PARAM_HEAD = PARAM_TAIL = NULL;

    if (TOKEN.token_type == T_SEMI) {
        match(T_SEMI, ";");
        return NULL;
    }

    FUNC_PTR = old_func;

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

    return make_ast_unary(A_FUNCTION, type, tree, old_func, end_label);
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
            reset_local_syms();
        } else {
            multi_declare_var(type, C_GLOBAL);
        }

        if (TOKEN.token_type == T_EOF) {
            break;
        }
    }
}
