//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static Symbol *declare_composite(int ptype);

int parse_type(Symbol **ctype) {
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
        case T_STRUCT:
            type = P_STRUCT;
            *ctype = declare_composite(P_STRUCT);
            break;
        case T_UNION:
            type = P_UNION;
            *ctype = declare_composite(P_UNION);
            break;
        default:
            fatald("Illegal type, token", TOKEN.token_type);
    }

    if (type != P_STRUCT && type != P_UNION) {
        scan();
    }

    while (TRUE) {
        if (TOKEN.token_type != T_STAR) {
            break;
        }
        type = pointer_to(type);
        scan();
    }

    return type;
}

Symbol *declare_var(int type, Symbol *ctype, int class) {
    Symbol *sym = NULL;

    switch (class) {
        case C_GLOBAL:
            if (find_global_sym(TEXT)) {
                fatals("Global variable redeclaration", TEXT);
            }
        case C_LOCAL:
        case C_PARAM:
            if (find_local_sym(TEXT)) {
                fatals("Local variable redeclaration", TEXT);
            }
        case C_MEMBER:
            if (find_member_sym(TEXT)) {
                fatals("Struct/Union redeclaration", TEXT);
            }
        default:
            break;
    }

    if (TOKEN.token_type == T_LBRACKET) {
        match(T_LBRACKET, "[");

        if (TOKEN.token_type == T_INTLIT) {
            switch (class) {
                case C_GLOBAL:
                    sym = add_global_sym(TEXT, pointer_to(type), ctype, S_ARRAY, (int) TOKEN.int_value);
                    break;
                case C_LOCAL:
                case C_PARAM:
                case C_MEMBER:
                    fatal("For now, declaration of non-global arrays is not implemented");
                default:
                    break;
            }
        }

        scan();
        match(T_RBRACKET, "]");
    } else {
        switch (class) {
            case C_GLOBAL:
                sym = add_global_sym(TEXT, type, ctype, S_VARIABLE, 1);
                break;
            case C_LOCAL:
                sym = add_local_sym(TEXT, type, ctype, S_VARIABLE, 1);
                break;
            case C_PARAM:
                sym = add_param_sym(TEXT, type, ctype, S_VARIABLE, 1);
                break;
            case C_MEMBER:
                sym = add_member_sym(TEXT, type, ctype, S_VARIABLE, 1);
                break;
            default:
                break;
        }
    }

    return sym;
}

static int declare_var_list(Symbol *func, int class, int separate_token, int end_token) {
    int type, param_cnt = 0;
    Symbol *ctype, *proto_ptr = NULL;

    if (func) {
        proto_ptr = func->first;
    }

    while (TOKEN.token_type != end_token) {
        type = parse_type(&ctype);
        match(T_IDENT, "identifier");

        if (proto_ptr) {
            if (type != proto_ptr->ptype) {
                fatald("Type mismatch of parameter", param_cnt);
            }
        } else {
            declare_var(type, ctype, class);
        }

        param_cnt++;

        if (TOKEN.token_type == separate_token) {
            match(separate_token, "separator");
        } else if (TOKEN.token_type != end_token) {
            fatald("Expected a separator or end token, but occurred", TOKEN.token_type);
        }
    }

    if (func && param_cnt != func->n_param) {
        fatals("Parameter count mismatch for function", func->name);
    }

    return param_cnt;
}

static int declare_params(Symbol *func) {
    return declare_var_list(func, C_PARAM, T_COMMA, T_RPAREN);
}

ASTnode *declare_func(int type) {
    ASTnode *tree, *final_stmt;
    Symbol *old_func, *new_func = NULL;
    int end_label, param_cnt;

    if ((old_func = find_global_sym(TEXT)) && old_func->stype != S_FUNCTION) {
        old_func = NULL;
    }

    if (!old_func) {
        end_label = gen_label();
        new_func = add_global_sym(TEXT, type, NULL, S_FUNCTION, end_label);
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

void multi_declare_var(int type, Symbol *ctype, int class) {
    while (TRUE) {
        declare_var(type, ctype, class);

        if (TOKEN.token_type == T_SEMI) {
            match(T_SEMI, ";");
            break;
        }

        match(T_COMMA, ",");
        match(T_IDENT, "identifier");
    }
}

static Symbol *declare_composite(int ptype) {
    Symbol *member, *ctype = NULL;
    int offset = 0;

    // skip struct/union keyword
    scan();

    // if a named composite
    if (TOKEN.token_type == T_IDENT) {
        if (ptype == P_STRUCT) {
            ctype = find_struct_sym(TEXT);
        } else {
            ctype = find_union_sym(TEXT);
        }
        match(T_IDENT, "identifier");
    }

    if (TOKEN.token_type != T_LBRACE) {
        if (!ctype) {
            fatals("Unknown struct/union type", TEXT);
        }
        return ctype;
    }

    // struct xxx{}
    if (ctype) {
        fatals("Struct type already defined", TEXT);
    }
    if (ptype == P_STRUCT) {
        ctype = add_struct_sym(TEXT, P_STRUCT, NULL, S_NONE, 0);
    } else {
        ctype = add_union_sym(TEXT, P_UNION, NULL, S_NONE, 0);
    }
    match(T_LBRACE, "{");
    declare_var_list(NULL, C_MEMBER, T_SEMI, T_RBRACE);
    match(T_RBRACE, "}");

    ctype->first = MEMBER_HEAD;
    MEMBER_HEAD = MEMBER_TAIL = NULL;

    if (ptype == P_STRUCT) { // for struct
        for (member = ctype->first; member; member = member->next) {
            member->posn = gen_align(member->ptype, offset, ASC);
            offset = member->posn + size_of_type(member->ptype, member->ctype);
        }
        ctype->size = offset;
    } else { // for union
        int cur_size, max_size = 0;
        for (member = ctype->first; member; member = member->next) {
            member->posn = 0;
            cur_size = size_of_type(member->ptype, member->ctype);
            max_size = max_size > cur_size ? max_size : cur_size;
        }
        ctype->size = max_size;
    }

    // DEBUG START
    // print struct info // TODO remove it later
    printf("[DEBUG]: struct/union %s's size is %d\n", ctype->name, ctype->size);
    for (Symbol *debug_member = ctype->first; debug_member; debug_member = debug_member->next) {
        printf("[DEBUG]: offset of %s.%s is %d\n", ctype->name, debug_member->name, debug_member->posn);
    }
    printf("\n");
    // DEBUG END

    return ctype;
}

void declare_global() {
    ASTnode *tree;
    Symbol *ctype;
    int type;

    while (TRUE) {
        type = parse_type(&ctype);

        if ((type == P_STRUCT || type == P_UNION) && TOKEN.token_type == T_SEMI) {
            match(T_SEMI, ";");
            continue;
        }

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

            gen_ast(tree, NO_LABEL, A_NONE);
            reset_local_syms();
        } else {
            multi_declare_var(type, ctype, C_GLOBAL);
        }

        if (TOKEN.token_type == T_EOF) {
            break;
        }
    }
}
