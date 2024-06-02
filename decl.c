//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static Symbol *declare_composite(int ptype);

static void declare_enum();

static int declare_typedef(Symbol **ctype);

static int type_of_typedef(char *name, Symbol **ctype);

int parse_type(Symbol **ctype) {
    int type = 0, can_no_var = FALSE;
    switch (TOKEN.token_type) {
        case T_VOID:
            type = P_VOID;
            match(T_VOID, "void");
            break;
        case T_CHAR:
            type = P_CHAR;
            match(T_CHAR, "char");
            break;
        case T_INT:
            type = P_INT;
            match(T_INT, "int");
            break;
        case T_LONG:
            type = P_LONG;
            match(T_LONG, "long");
            break;
        case T_STRUCT:
            type = P_STRUCT;
            *ctype = declare_composite(P_STRUCT);
            break;
        case T_UNION:
            type = P_UNION;
            *ctype = declare_composite(P_UNION);
            break;
        case T_ENUM:
            type = P_INT;
            declare_enum();
            can_no_var = TRUE;
            break;
        case T_TYPEDEF:
            type = declare_typedef(ctype);
            can_no_var = TRUE;
            break;
        case T_IDENT:
            type = type_of_typedef(TEXT, ctype);
            break;
        default:
            fatald("Illegal type, token", TOKEN.token_type);
    }

    while (TRUE) {
        if (TOKEN.token_type != T_STAR) {
            break;
        }
        type = pointer_to(type);
        can_no_var = FALSE;
        scan();
    }

    // No variable name follows
    if (TOKEN.token_type == T_SEMI) {
        // not a variable declaration, but only a struct/union/enum/typedef definition
        if (type == P_STRUCT || type == P_UNION || can_no_var) {
            type = P_NONE;
        } else {
            fatal("Variable name expected");
        }
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

        if (TOKEN.token_type == end_token) {
            // if a struct or union
            if (!func) {
                warning("No semicolon at end of struct or union");
            }
            break;
        }
        match(separate_token, "separator");
        if (TOKEN.token_type == end_token) {
            break;
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

#ifdef DEBUG
    // DEBUG START
    // print struct info
    printf("[DEBUG] struct/union %s's size is %d\n", ctype->name, ctype->size);
    for (Symbol *debug_member = ctype->first; debug_member; debug_member = debug_member->next) {
        printf("[DEBUG] offset of %s.%s is %d\n", ctype->name, debug_member->name, debug_member->posn);
    }
    printf("\n");
    // DEBUG END
#endif

    return ctype;
}

static void declare_enum() {
    Symbol *enum_sym = NULL;
    char *name;
    int val = 0;

    // skip enum keyword
    scan();

    if (TOKEN.token_type == T_IDENT) {
        enum_sym = find_enum_type_sym(TEXT);
        name = strdup(TEXT);
        match(T_IDENT, "identifier");
    }

    if (TOKEN.token_type != T_LBRACE) {
        if (!enum_sym) {
            fatals("Unknown enum type", name);
        }
        return;
    }

    // then must be {
    // we are declare a new enum type
    match(T_LBRACE, "{");
    if (enum_sym) {
        fatals("Enum type already defined", enum_sym->name);
    }
    add_enum_sym(name, C_ENUMTYPE, 0);

    while (TRUE) {
        match(T_IDENT, "identifier"); // must have a name
        name = strdup(TEXT);

        enum_sym = find_enum_val_sym(TEXT);
        if (enum_sym) {
            fatals("Enum value already defined", TEXT);
        }

        if (TOKEN.token_type == T_ASSIGN) {
            match(T_ASSIGN, "=");
            if (TOKEN.token_type != T_INTLIT) {
                fatal("Enum value must be integer literal");
            }

            val = (int) TOKEN.int_value;
            match(T_INTLIT, "integer literal");
        }

        add_enum_sym(name, C_ENUMVAL, val++);

        if (TOKEN.token_type == T_RBRACE) {
            break;
        }
        match(T_COMMA, ",");
        if (TOKEN.token_type == T_RBRACE) {
            break;
        }
    }

    match(T_RBRACE, "}");
}

static int declare_typedef(Symbol **ctype) {
    int type;

    // skip typedef keyword
    match(T_TYPEDEF, "typedef");

    type = parse_type(ctype);

    if (TOKEN.token_type != T_IDENT) {
        fatal("Typedef type must be identifier");
    }
    if (find_typedef_sym(TEXT)) {
        fatals("Typedef type already defined", TEXT);
    }

    add_typedef_sym(TEXT, type, *ctype, S_NONE, 0);

    return type;
}

static int type_of_typedef(char *name, Symbol **ctype) {
    Symbol *type;

    if (!(type = find_typedef_sym(name))) {
        fatals("Unknown type", name);
    }

    // skip type name
    match(T_IDENT, "identifier");
    *ctype = type->ctype;

    return type->ptype;
}

void declare_global() {
    ASTnode *tree;
    Symbol *ctype;
    int type;

    while (TRUE) {
        type = parse_type(&ctype);

        if (type == P_NONE) {
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
