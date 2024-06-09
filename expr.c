//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

static int op_prec[] = {
        0, 10, 10, 10, 10, 10,  // T_EOF, T_ASSIGN, T_ASPLUS, T_ASMINUS, T_ASPSTAR, T_ASPFSLASH
        15,                     // T_QUESTION
        20, 30,                 // T_LOGOR, T_LOGAND
        40, 50, 60,             // T_OR, T_XOR, T_AND
        70, 70,                 // T_EQ, T_NE
        80, 80, 80, 80,         // T_LT, T_GT, T_LE, T_GE
        90, 90,                 // T_LSHIFT, T_RSHIFT
        100, 100,               // T_PLUS, T_MINUS
        110, 110,               // T_STAR, T_SLASH
};

ASTnode *expression_list(int end_token) {
    ASTnode *node = NULL, *param_node = NULL;
    int param_cnt = 0;

    while (TOKEN.token_type != end_token) {
        param_node = bin_expr(0);
        node = make_ast_node(A_GLUE, P_NONE, node, NULL, param_node, NULL, param_cnt++);

        if (TOKEN.token_type == end_token) {
            break;
        }
        match(T_COMMA, ",");
    }

    return node;
}

ASTnode *func_call() {
    Symbol *func;

    if (!(func = find_global_sym(TEXT)) || func->stype != S_FUNCTION) {
        fatals("Undeclared function", TEXT);
    }
    scan();
    ASTnode *node = expression_list(T_RPAREN);
    scan();
    // TODO check params against prototype
    node = make_ast_unary(A_FUNCCALL, func->ptype, node, func, 0);
    return node;
}

static ASTnode *access_array() {
    ASTnode *left, *right;
    Symbol *arr;

    if (!(arr = find_sym(TEXT))) {
        fatals("Undeclared array", TEXT);
    }
    if (arr->stype != S_ARRAY && (arr->stype == S_VARIABLE && !is_ptr(arr->ptype))) {
        fatals("Not an array or pointer", TEXT);
    }

    if (arr->stype == S_ARRAY) {
        left = make_ast_leaf(A_ADDR, arr->ptype, arr, 0);
    } else {
        left = make_ast_leaf(A_IDENT, arr->ptype, arr, 0);
        left->rvalue = TRUE;
    }

    scan();
    right = bin_expr(0);
    match(T_RBRACKET, "]");

    if (!is_int(right->type)) {
        fatal("Array index must be an integer");
    }

    right = modify_type(right, left->type, A_ADD);
    left = make_ast_node(A_ADD, arr->ptype, left, NULL, right, NULL, 0);
    left = make_ast_unary(A_DEREF, value_at(left->type), left, NULL, 0);

    return left;
}

static ASTnode *access_member(int with_pointer) {
    ASTnode *left, *right;
    Symbol *var, *ctype, *member;

    // check variable type
    if (!(var = find_sym(TEXT)) || var->stype != S_VARIABLE) {
        fatals("Unknown variable", TEXT);
    }
    if (!with_pointer && var->ptype != P_STRUCT && var->ptype != P_UNION) {
        fatals("Can't access member of non-struct type", TEXT);
    }
    if (with_pointer && var->ptype != pointer_to(P_STRUCT) && var->ptype != pointer_to(P_UNION)) {
        fatals("Can't access member of non-struct type", TEXT);
    }

    // get var's pointer
    if (with_pointer) {
        left = make_ast_leaf(A_IDENT, pointer_to(var->ptype), var, 0);
    } else {
        left = make_ast_leaf(A_ADDR, var->ptype, var, 0);
    }
    left->rvalue = TRUE;
    ctype = var->ctype;

    scan();
    match(T_IDENT, "identifier");

    // get member name
    for (member = ctype->first; member; member = member->next) {
        if (!strcmp(member->name, TEXT)) {
            break;
        }
    }
    if (!member) {
        fatals("No member found in struct/union", TEXT);
    }

    // get member offset
    right = make_ast_leaf(A_INTLIT, P_INT, NULL, member->posn);
    // add offset to pointer
    left = make_ast_node(A_ADD, pointer_to(member->ptype), left, NULL, right, NULL, 0);
    // dereference pointer to get member value
    left = make_ast_unary(A_DEREF, member->ptype, left, NULL, 0);

    return left;
}

static ASTnode *postfix() {
    ASTnode *node;
    Symbol *var;
    int rvalue = FALSE;

    if ((var = find_enum_val_sym(TEXT))) {
        scan();
        return make_ast_leaf(A_INTLIT, P_INT, NULL, var->posn);
    }

    switch (peek_token().token_type) {
        case T_LPAREN:
            scan();
            return func_call();
        case T_LBRACKET:
            scan();
            return access_array();
        case T_DOT:
            scan();
            return access_member(FALSE);
        case T_ARROW:
            scan();
            return access_member(TRUE);
    }

    if (!(var = find_sym(TEXT))) {
        fatals("Unknown variable", TEXT);
    }
    switch (var->stype) {
        case S_VARIABLE:
            break;
        case S_ARRAY:
            rvalue = TRUE;
            break;
        default:
            fatals("Not a variable", TEXT);
    }

    scan();

    switch (TOKEN.token_type) {
        case T_INC:
            if (rvalue) {
                fatals("Can't increment rvalue", TEXT);
            }
            scan();
            node = make_ast_leaf(A_POSTINC, var->ptype, var, 0);
            break;
        case T_DEC:
            if (rvalue) {
                fatals("Can't decrement rvalue", TEXT);
            }
            scan();
            node = make_ast_leaf(A_POSTDEC, var->ptype, var, 0);
            break;
        default:
            if (rvalue) {
                node = make_ast_leaf(A_ADDR, var->ptype, var, 0);
                node->rvalue = TRUE;
            } else {
                node = make_ast_leaf(A_IDENT, var->ptype, var, 0);
            }
    }

    return node;
}

static ASTnode *primary() {
    ASTnode *node = NULL;
    int label, size, class, type = P_NONE;
    Symbol *ctype, *sym;

    switch (TOKEN.token_type) {
        case T_STATIC:
        case T_EXTERN:
            fatal("For now, compiler not support static local variable");
        case T_SIZEOF:
            scan();
            match(T_LPAREN, "( after sizeof");

            if ((sym = find_sym(TEXT))) {
                scan();
                size = sym->size;
            } else {
                type = parse_type(&ctype, &class);
                type = parse_stars(type);
                size = size_of_type(type, ctype);
            }

            match(T_RPAREN, "closed ) sizeof");
            return make_ast_leaf(A_INTLIT, P_INT, NULL, size);
        case T_INTLIT:
            if (TOKEN.int_value >= 0 && TOKEN.int_value <= 0xff) {
                node = make_ast_leaf(A_INTLIT, P_CHAR, NULL, TOKEN.int_value);
            } else if (TOKEN.int_value <= 0xffff) {
                node = make_ast_leaf(A_INTLIT, P_SHORT, NULL, TOKEN.int_value);
            } else {
                node = make_ast_leaf(A_INTLIT, P_INT, NULL, TOKEN.int_value);
            }
            break;
        case T_STRLIT:
            label = gen_new_str(TEXT);
            node = make_ast_leaf(A_STRLIT, pointer_to(P_CHAR), NULL, label);
            break;
        case T_IDENT:
            return postfix();
        case T_LPAREN:
            scan();

            switch (TOKEN.token_type) {
                case T_IDENT:
                    if (!find_typedef_sym(TEXT)) {
                        node = bin_expr(0);
                        break;
                    }
                case T_VOID:
                case T_CHAR:
                case T_SHORT:
                case T_INT:
                case T_LONG:
                case T_STRUCT:
                case T_UNION:
                case T_ENUM:
                    type = parse_cast();
                    match(T_RPAREN, ")");
                default:
                    node = bin_expr(0);
            }

            if (type == P_NONE) {
                match(T_RPAREN, ")");
            } else {
                node = make_ast_unary(A_CAST, type, node, NULL, 0);
            }

            return node;
        default:
            fatals("syntax error, token", get_name(V_TOKEN, TOKEN.token_type));
    }

    scan();
    return node;
}

static int token_to_op(int tk) {
    if (tk > T_EOF && tk < T_SLASH) {
        return tk;
    }
    fatals("Unknown token", get_name(V_TOKEN, TOKEN.token_type));
    return 0;
}

static int stick_right(int token_type) {
    if (token_type >= T_ASSIGN && token_type <= T_ASSLASH) {
        return TRUE;
    }
    return FALSE;
}

static int op_precedence(int token_type) {
    if (token_type == T_VOID) {
        fatals("Token with no precendence", get_name(V_TOKEN, token_type));
    }

    int prec = op_prec[token_type];
    if (prec == 0) {
        fatals("Syntax error, token type", get_name(V_TOKEN, token_type));
    }
    return prec;
}

static ASTnode *prefix() {
    ASTnode *tree;

    switch (TOKEN.token_type) {
        case T_AND:
            scan();
            tree = prefix();

            // if (tree->op != A_IDENT) {
            //     fatal("& operator must be followed by a variable");
            // }

            if (tree->op == A_IDENT) {
                tree->op = A_ADDR;
                tree->type = pointer_to(tree->type);
            }
            break;
        case T_STAR:
            scan();
            tree = prefix();

            if (tree->op != A_IDENT && tree->op != A_DEREF) {
                fatal("* operator must be followed by a variable or *");
            }

            tree = make_ast_unary(A_DEREF, value_at(tree->type), tree, NULL, 0);
            break;
        case T_MINUS:
            scan();
            tree = prefix();

            tree->rvalue = TRUE;
            tree = modify_type(tree, P_INT, P_NONE);
            tree = make_ast_unary(A_NEGATE, tree->type, tree, NULL, 0);
            break;
        case T_INVERT:
            scan();
            tree = prefix();

            tree->rvalue = TRUE;
            tree = make_ast_unary(A_INVERT, tree->type, tree, NULL, 0);
            break;
        case T_LOGNOT:
            scan();
            tree = prefix();

            tree->rvalue = TRUE;
            tree = make_ast_unary(A_LOGNOT, tree->type, tree, NULL, 0);
            break;
        case T_INC:
            scan();
            tree = prefix();

            if (tree->op != A_IDENT) {
                fatal("++ operator must be followed by a variable");
            }
            tree = make_ast_unary(A_PREINC, tree->type, tree, NULL, 0);
            break;
        case T_DEC:
            scan();
            tree = prefix();

            if (tree->op != A_IDENT) {
                fatal("++ operator must be followed by a variable");
            }
            tree = make_ast_unary(A_PREDEC, tree->type, tree, NULL, 0);
            break;
        case T_PLUS:
            scan();
        default:
            tree = primary();
    }

    return tree;
}

/*
 * @param ptp: previous token precedence
 */
ASTnode *bin_expr(int ptp) {
    ASTnode *ltemp, *rtemp, *right, *left = prefix();
    int ast_op, token_type = TOKEN.token_type;

    switch (token_type) {
        case T_SEMI:
        case T_RPAREN:
        case T_RBRACKET:
        case T_COMMA:
        case T_COLON:
        case T_RBRACE:
            left->rvalue = TRUE;
            return left;
        default:
            break;
    }

    while (op_precedence(token_type) > ptp
           || (stick_right(token_type) && op_precedence(token_type) == ptp)) {
        scan();

        right = bin_expr(op_prec[token_type]);

        ast_op = token_to_op(token_type);
        switch (ast_op) {
            case A_ASSIGN:
                right->rvalue = TRUE;

                right = modify_type(right, left->type, P_NONE);
                if (!right) {
                    fatal("Incompatible expression in assignment");
                }
                // swap left and right
                ltemp = left;
                left = right;
                right = ltemp;
                break;
            case A_TERNARY:
                match(T_COLON, "':' after conditional expression");
                ltemp = bin_expr(0);

                return make_ast_node(ast_op, right->type, left, right, ltemp, NULL, 0);
            default:
                left->rvalue = TRUE;
                right->rvalue = TRUE;

                ltemp = modify_type(left, right->type, ast_op);
                rtemp = modify_type(right, left->type, ast_op);
                if (!ltemp && !rtemp) {
                    fatal("Incompatible types in arithmetic expression");
                }

                if (ltemp) {
                    left = ltemp;
                }
                if (rtemp) {
                    right = rtemp;
                }
        }

        left = make_ast_node(ast_op, left->type, left, NULL, right, NULL, 0);

        token_type = TOKEN.token_type;
        switch (token_type) {
            case T_SEMI:
            case T_RPAREN:
            case T_RBRACKET:
            case T_COMMA:
            case T_COLON:
            case T_RBRACE:
                left->rvalue = TRUE;
                return left;
            default:
                break;
        }
    }

    left->rvalue = TRUE;
    return left;
}
