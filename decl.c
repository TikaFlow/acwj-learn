//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static int not_typedef = TRUE;

static Symbol *declare_composite(int ptype);

static void declare_enum();

static int declare_typedef(Symbol **ctype);

static int type_of_typedef(char *name, Symbol **ctype);

int parse_type(Symbol **ctype, int *class) {
    int type = P_NONE, can_no_var = FALSE, storage_flag = TRUE;

    while (storage_flag) {
        switch (TOKEN.token_type) {
            case T_EXTERN:
                if (*class == C_STATIC) {
                    fatal("'extern' not allowed in static declaration");
                }
                *class = C_EXTERN;
                scan();
                break;
            case T_STATIC:
                if (*class == C_EXTERN) {
                    fatal("'static' not allowed in extern declaration");
                } else if (*class == C_LOCAL) {
                    fatal("For now, compiler not support static local variable");
                }
                *class = C_STATIC;
                scan();
                break;
            default:
                storage_flag = FALSE;
        }
    }

    switch (TOKEN.token_type) {
        case T_VOID:
            type = P_VOID;
            scan();
            break;
        case T_CHAR:
            type = P_CHAR;
            scan();
            break;
        case T_SHORT:
            type = P_SHORT;
            scan();
            break;
        case T_INT:
            type = P_INT;
            scan();
            break;
        case T_LONG:
            type = P_LONG;
            scan();
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
            fatals("Illegal type, token", get_name(V_TOKEN, TOKEN.token_type));
    }

    // No variable name follows
    if (TOKEN.token_type == T_SEMI) {
        // not a variable declaration, but only a struct/union/enum/typedef definition
        if (type == P_STRUCT || type == P_UNION || can_no_var) {
        } else {
            warning("Declaration does not declare anything");
        }
        type = P_NONE;
        scan();
    }

    return type;
}

int parse_stars(int type) {
    while (TRUE) {
        if (TOKEN.token_type != T_STAR) {
            break;
        }
        type = pointer_to(type);
        scan();
    }

    return type;
}

int parse_cast(Symbol **ctype) {
    int type, class = C_NONE;

    type = parse_type(ctype, &class);
    if (type == P_NONE) {
        return type;
    }

    type = parse_stars(type);

    if (type == P_STRUCT || type == P_UNION || type == P_VOID) {
        fatal("Cannot cast to struct, union or void type");
    }

    return type;
}

static long parse_literal(int type) {
    ASTnode *tree = optimize(bin_expr(0));

    if (tree->op == A_CAST) {
        tree->left->type = tree->type;
        tree = tree->left;
    }

    if (tree->op != A_INTLIT && tree->op != A_STRLIT) {
        fatal("Cannot initialize globals with a generic expression");
    }

    if (type == pointer_to(P_CHAR)) {
        if (tree->op == A_STRLIT) {
            return tree->int_value;
        }
        if (tree->op == A_INTLIT && tree->int_value == 0) {
            return 0;
        }
    }

    if (is_int(type) && size_of_type(type, NULL) >= size_of_type(tree->type, NULL)) {
        return tree->int_value;
    }

    fatal("Type mismatch in initializer");
    return 0; // keep compiler happy
}

static int is_new_sym(Symbol *sym, int class, int type, Symbol *ctype) {
    if (!sym) {
        return TRUE;
    }

    if ((sym->class == C_GLOBAL && class == C_EXTERN)
        || (sym->class == C_EXTERN && class == C_GLOBAL)) {
        if (sym->ptype != type
            || (type >= P_STRUCT && sym->ctype != ctype)) {
            fatals("Type mismatch for symbol", sym->name);
        }

        sym->class = C_GLOBAL;
        return FALSE;
    }

    fatals("Duplicate global symbol declaration", sym->name);
    return FALSE; // keep compiler happy
}

static Symbol *declare_array(char *name, int type, Symbol *ctype, int class) {
    Symbol *sym = NULL;
    int n_elem = -1;

    // skip '['
    scan();

    if (TOKEN.token_type != T_RBRACKET) {
        n_elem = parse_literal(P_INT);
        if (n_elem <= 0) {
            fatald("Array size must be positive, but get", n_elem);
        }
    }

    // check ']'
    match(T_RBRACKET, "]");

    //treat arr name as a pointer th its elements' type
    switch (class) {
        case C_STATIC:
        case C_EXTERN:
        case C_GLOBAL:
            sym = find_global_sym(name);
            if (is_new_sym(sym, class, pointer_to(type), ctype)) {
                sym = add_global_sym(name, pointer_to(type), ctype, S_ARRAY, class, n_elem, 0);
            }
            break;
        case C_LOCAL:
            sym = add_local_sym(name, pointer_to(type), ctype, S_ARRAY, n_elem);
            break;
        case C_PARAM:
        case C_MEMBER:
            fatal("TODO: declare_array for non-global");
            break;
        default:
            break; // keep compiler happy
    }

    return sym;
}

static Symbol *declare_scalar(char *name, int type, Symbol *ctype, int class, Symbol **head, Symbol **tail) {
    Symbol *sym = NULL;
    switch (class) {
        case C_STATIC:
        case C_EXTERN:
        case C_GLOBAL:
            sym = find_global_sym(name);
            if (is_new_sym(sym, class, type, ctype)) {
                sym = add_global_sym(name, type, ctype, S_VARIABLE, class, 1, 0);
            }
            break;
        case C_LOCAL:
            sym = add_local_sym(name, type, ctype, S_VARIABLE, 1);
            break;
        case C_PARAM:
            sym = add_param_sym(name, type, ctype, S_VARIABLE);
            break;
        case C_MEMBER:
            sym = add_member_sym(name, head, tail, type, ctype, S_VARIABLE, 1);
            break;
        default:
            break; // keep compiler happy
    }

    return sym;
}

static int declare_param_list(Symbol *old_func, Symbol *new_func) {
    int type, param_cnt = 0;
    Symbol *ctype, *proto_ptr = NULL;
    ASTnode *unused;

    if (old_func) {
        proto_ptr = old_func->first;
    }

    while (TOKEN.token_type != T_RPAREN) {
        // check (void)
        if (TOKEN.token_type == T_VOID) {
            if (peek_token().token_type == T_RPAREN) {
                param_cnt = 0;
                scan();
                break;
            }
        }

        type = declare_list(&ctype, C_PARAM, T_COMMA, T_RPAREN, &unused, NULL, NULL);
        if (type == P_NONE) {
            fatal("Bad type in parameter list");
        }

        if (proto_ptr) {
            if (type != proto_ptr->ptype) {
                fatald("Type mismatch of parameter", param_cnt);
            }
        }

        param_cnt++;

        if (TOKEN.token_type == T_RPAREN) {
            break;
        }
        match(T_COMMA, ",");
    }

    if (old_func && param_cnt != old_func->n_elem) {
        fatals("Parameter count mismatch for function", old_func->name);
    }

    return param_cnt;
}

static Symbol *declare_func(char *name, int type, Symbol *ctype, int class) {
    ASTnode *tree, *final_stmt;
    Symbol *old_func, *new_func = NULL;
    int end_label = 0, param_cnt;

    if ((old_func = find_global_sym(name)) && old_func->stype != S_FUNCTION) {
        old_func = NULL;
    }

    if (!old_func) {
        end_label = gen_label();
        new_func = add_global_sym(name, type, ctype, S_FUNCTION, class, 0, end_label);
    }

    match(T_LPAREN, "(");
    param_cnt = declare_param_list(old_func, new_func);
    match(T_RPAREN, ")");

    if (new_func) {
        new_func->n_elem = param_cnt;
        new_func->first = PARAM_HEAD;
        old_func = new_func;
    }
    PARAM_HEAD = PARAM_TAIL = NULL;

    // only a prototype
    if (TOKEN.token_type == T_SEMI) {
        return old_func;
    }

    FUNC_PTR = old_func;

    SWITCH_LEVEL = 0;
    LOOP_LEVEL = 0;

    // function body must start with {
    if (TOKEN.token_type != T_LBRACE) {
        fatal("No '{' in function body");
    }
    tree = compound_stmt(FALSE);

    if (type != P_VOID) {
        if (!tree) {
            fatal("No statements in function with non-void type");
        }

        final_stmt = tree->op == A_GLUE ? tree->right : tree;
        if (!final_stmt || final_stmt->op != A_RETURN) {
            fatal("No return statement in function with non-void return type");
        }
    }

    // optimize
    tree = optimize(tree);

    tree = make_ast_unary(A_FUNCTION, type, ctype, tree, old_func, end_label);

    gen_ast(tree, NO_LABEL, NO_LABEL, NO_LABEL, A_NONE);
    reset_local_syms();

    return old_func;
}

static Symbol *declare_composite(int ptype) {
    Symbol *member = NULL, *ctype = NULL;
    ASTnode *unused;
    int type, cur_size, max_size = 0, mem_cnt = 0;
    char *name = NULL;

    // skip struct/union keyword
    scan();

    // if a named composite
    if (TOKEN.token_type == T_IDENT) {
        name = strdup(TEXT);
        if (ptype == P_STRUCT) {
            ctype = find_struct_sym(name);
        } else {
            ctype = find_union_sym(name);
        }
        scan();
    }

    if (ctype) {
        // we found a existing one, but now defining a new composite
        if (TOKEN.token_type == T_LBRACE && ctype->size > 0) {
            fatals("Struct/Union type already defined", TEXT);
        }
    } else {
        if (ptype == P_STRUCT) {
            ctype = add_struct_sym(name);
        } else {
            ctype = add_union_sym(name);
        }
        ctype->size = -1; // mark as half definition
    }

    /*
     * after struct xxx, should be one of below:
     * ; - half definition
     * { - full definition
     * * - pointer to composite
     * ident
     *      - variable/param/function name
     *      - typedef
     *
     * if not defining a new composite, and also not half definition, then must define
     * something of this type which should have full definition
     */
    if (TOKEN.token_type != T_LBRACE) {
        if (TOKEN.token_type != T_SEMI && ctype->size < 0 && not_typedef) {
            fatals("Incomplete type declaration", name);
        }
        return ctype;
    }

    // struct xxx{}
    scan();
    // parse member list
    Symbol *mem_head = NULL, *mem_tail = NULL;
    int invalid = FALSE, nothing, composite;
    ctype->size = 0; // mark as full defined
    while (TRUE) {
        nothing = composite = FALSE;
        type = declare_list(&member, C_MEMBER, T_SEMI, T_RBRACE, &unused, &mem_head, &mem_tail);
        if (type == P_NONE) { // if no member name
            // a nested struct/union
            if (member && (member->ptype == P_STRUCT || member->ptype == P_UNION)) {
                composite = TRUE;
                if (member->name) {
                    nothing = TRUE;
                } else {
                    add_member_sym(member->name, &mem_head, &mem_tail, member->ptype, member, S_VARIABLE, 1);
                }
            } else {
                invalid = TRUE;
            }
        }
        if (nothing) {
            warning("Declaration does not declare anything");
        }
        if (invalid) {
            fatal("Declaration does not declare anything");
        }

        mem_cnt++;
        if (TOKEN.token_type == T_RBRACE) {
            if (!composite) {
                warning("No semicolon at end of struct or union");
            }
            break;
        }
        if (!composite) {
            match(T_SEMI, "separator");
        }
        if (TOKEN.token_type == T_RBRACE) {
            break;
        }
    }
    match(T_RBRACE, "}");

    if (!mem_cnt) {
        fatal("No member in struct/union");
    }

    ctype->first = mem_head;

    // calculate offset and size
    if (ptype == P_STRUCT) { // for struct
        for (member = ctype->first; member; member = member->next) {
            member->posn = gen_align(member->ptype, max_size, ASC);
            cur_size = size_of_type(member->ptype, member->ctype);
            max_size = member->posn + (cur_size < 8 ? 8 : cur_size);
        }
    } else { // for union
        for (member = ctype->first; member; member = member->next) {
            member->posn = 0;
            cur_size = size_of_type(member->ptype, member->ctype);
            cur_size = cur_size < 8 ? 8 : cur_size;
            max_size = max_size > cur_size ? max_size : cur_size;
        }
    }
    ctype->size = max_size;

#ifdef DEBUG
    // print struct info
    debug(format_str("%s %s's size is %d", ptype == P_STRUCT ? "struct" : "union", ctype->name, ctype->size));
    for (Symbol *debug_member = ctype->first; debug_member; debug_member = debug_member->next) {
        debug(format_str("offset of %s.%s is %d", ctype->name, debug_member->name, debug_member->posn));
    }
    printf("\n");
#endif // DEBUG

    return ctype;
}

static void declare_enum() {
    Symbol *enum_sym = NULL;
    char *name = NULL;
    int val = 0;

    // skip enum keyword
    scan();

    if (TOKEN.token_type == T_IDENT) {
        enum_sym = find_enum_type_sym(TEXT);
        name = strdup(TEXT);
        scan();
    }

    if (TOKEN.token_type != T_LBRACE) {
        if (!enum_sym) {
            fatals("Unknown enum type", name);
        }
        return;
    }

    // then must be {
    // we are declare a new enum type
    scan();
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
            scan();
            val = (int) parse_literal(P_INT);
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
    int type, class = C_NONE;

    not_typedef = FALSE;
    // skip typedef keyword
    scan();

    type = parse_type(ctype, &class);
    type = parse_stars(type);

    if (class == C_EXTERN) {
        fatal("Typedef type cannot be used with extern");
    }

    if (TOKEN.token_type != T_IDENT) {
        fatal("Typedef type must be identifier");
    }
    if (find_typedef_sym(TEXT)) {
        fatals("Typedef type already defined", TEXT);
    }

    add_typedef_sym(TEXT, type, *ctype);
    // skip alias name
    scan();
    not_typedef = TRUE;

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

static void init_array(Symbol *sym, int type, Symbol *ctype, int class) {
    int n_elem = sym->n_elem, max_elem, i = 0, j;
    long *init_list;

    if (class == C_EXTERN || class == C_GLOBAL || class == C_STATIC) {
        // array init with {
        match(T_LBRACE, "{");

        if (n_elem == -1) {
            max_elem = DEFAULT_ARRAY_SIZE;
        } else {
            max_elem = n_elem;
        }
        init_list = (long *) malloc(sizeof(long) * max_elem);

        while (TRUE) {
            if (i >= max_elem) {
                if (n_elem == -1) {
                    max_elem += DEFAULT_ARRAY_SIZE;
                    init_list = (long *) realloc(init_list, sizeof(long) * max_elem);
                } else {
                    fatals("Array initialization too long", NULL);
                }
            }

            init_list[i++] = parse_literal(type);

            if (TOKEN.token_type == T_RBRACE) {
                break;
            }
            match(T_COMMA, ",");
            if (TOKEN.token_type == T_RBRACE) {
                break;
            }
        }
        if (n_elem == -1) {
            n_elem = i;
            init_list = (long *) realloc(init_list, sizeof(long) * i);
        }

        match(T_RBRACE, "}");

        // zero unused elements
        for (j = i; j < n_elem; j++) {
            init_list[j] = 0;
        }

        sym->init_list = init_list;
        sym->n_elem = n_elem;
        sym->size = n_elem * size_of_type(type, ctype);
    } else {
        // TODO: initialize local array
    }
}

static void init_var(Symbol *sym, int type, Symbol *ctype, int class, ASTnode **glue_tree) {
    ASTnode *var, *expr;
    *glue_tree = NULL;

    if (class == C_EXTERN || class == C_GLOBAL || class == C_STATIC) {
        sym->init_list = (long *) malloc(sizeof(long));
        sym->init_list[0] = parse_literal(type);
        sym->n_elem = 1;
    } else if (class == C_LOCAL) {
        var = make_ast_leaf(A_IDENT, sym->ptype, sym->ctype, sym, 0);
        expr = bin_expr(0);
        expr->rvalue = TRUE;

        expr = modify_type(expr, var, P_NONE);
        if (!expr) {
            fatal("Incompatible types in initialization");
        }
        *glue_tree = make_ast_node(A_ASSIGN, expr->type, expr->ctype, expr, NULL, var, NULL, 0);
    }
}

static Symbol *declare_sym(int type, Symbol *ctype, int class, ASTnode **glue_tree, Symbol **head, Symbol **tail) {
    Symbol *sym = NULL;
    char *name = strdup(TEXT);
    int stype = S_VARIABLE;
    *glue_tree = NULL;

    // should have a variable name
    match(T_IDENT, "identifier");

    // if a function
    if (TOKEN.token_type == T_LPAREN) {
        return declare_func(name, type, ctype, class);
    }

    // check if has been declared
    switch (class) {
        case C_STATIC:
        case C_EXTERN:
        case C_GLOBAL:
        case C_LOCAL:
            break;
        case C_PARAM:
            if (find_local_sym(name)) {
                fatals("Local variable already defined", name);
            }
            break;
        case C_MEMBER:
            if (find_member_sym(name, *head)) {
                fatals("Struct/Union member already defined", name);
            }
            break;
        default:
            break; // keep compiler happy
    }

    // add to symbol table
    if (TOKEN.token_type == T_LBRACKET) {
        sym = declare_array(name, type, ctype, class);
        stype = S_ARRAY;
    } else {
        sym = declare_scalar(name, type, ctype, class, head, tail);
    }

    // check if has a initial value
    if (TOKEN.token_type == T_ASSIGN) {
        // parameter or member has no initial value
        if (class == C_PARAM || class == C_MEMBER) {
            fatals("Parameter or member initialisation permitted", name);
        }

        // skip '='
        scan();

        // parse the initial value
        if (stype == S_ARRAY) {
            init_array(sym, type, ctype, class);
        } else {
            init_var(sym, type, ctype, class, glue_tree);
        }
    }

    // generate space if is a GLOBAL symbol
    if (class == C_GLOBAL || class == C_STATIC) {
        gen_new_sym(sym);
    }

    return sym;
}

int
declare_list(Symbol **ctype, int class, int end_tk1, int end_tk2, ASTnode **glue_tree, Symbol **head, Symbol **tail) {
    int init_type, type;
    Symbol *sym;
    ASTnode *tree = NULL;
    *glue_tree = NULL;

    if ((init_type = parse_type(ctype, &class)) == P_NONE) {
        return P_NONE;
    }

    while (TRUE) {
        // see if this symbol is a pointer
        type = parse_stars(init_type);

        // parse this symbol
        sym = declare_sym(type, *ctype, class, &tree, head, tail);

        if (tree) { // avoid unnecessary nesting
            if (*glue_tree) {
                *glue_tree = make_ast_node(A_GLUE, P_NONE, NULL, *glue_tree, NULL, tree, NULL, 0);
            } else {
                *glue_tree = tree;
            }
        }

        // if a func, no list, so leave
        if (sym->stype == S_FUNCTION) {
            if (class != C_GLOBAL && class != C_STATIC) {
                fatal("Function definition not at global scope");
            }
            return type;
        }

        // if this is the end of the list, return the type
        if (TOKEN.token_type == end_tk1 || TOKEN.token_type == end_tk2) {
            return type;
        }

        // if this is a comma, continue
        match(T_COMMA, ",");
    }
}

void declare_global() {
    Symbol *ctype = NULL;
    ASTnode *unused;

    while (TOKEN.token_type != T_EOF) {
        declare_list(&ctype, C_GLOBAL, T_SEMI, T_EOF, &unused, NULL, NULL);

        if (TOKEN.token_type == T_SEMI) {
            scan();
        }
    }
}
