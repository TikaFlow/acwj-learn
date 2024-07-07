//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

// T_EOF,
// T_ASSIGN, T_ASPLUS, T_ASMINUS, T_ASSTAR, T_ASSLASH, T_ASMOD, // 6
// T_QUESTION, T_LOGOR, T_LOGAND, T_OR, T_XOR, T_AND, // 12
// T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE, // 18
// T_LSHIFT, T_RSHIFT, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_MOD, // 25
static int op_prec[] = {
        0,
        1, 1, 1, 1, 1, 1,
        2, 3, 4, 5, 6, 7,
        8, 8, 9, 9, 9, 9,
        10, 10, 11, 11, 12, 12, 12,
};

ASTnode *expression_list(int end_token) {
    ASTnode *node = NULL, *param_node = NULL;
    int param_cnt = 0;

    while (TOKEN.token_type != end_token) {
        param_node = bin_expr(0);
        node = make_ast_node(A_GLUE, P_NONE, NULL, node, NULL, param_node, NULL, param_cnt++);

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
    node = make_ast_unary(A_FUNCCALL, func->ptype, func->ctype, node, func, 0);
    return node;
}

static ASTnode *access_array(ASTnode *left) {
    ASTnode *right;

    if (!is_ptr(left->type)) {
        fatals("Not an array or pointer", TEXT);
    }

    scan();
    right = bin_expr(0);
    match(T_RBRACKET, "]");
    left->rvalue = TRUE;

    if (!is_int(right->type)) {
        fatal("Array index must be an integer");
    }

    right = modify_type(right, left, A_ADD);
    left = make_ast_node(A_ADD, left->type, left->ctype, left, NULL, right, NULL, 0);
    left = make_ast_unary(A_DEREF, value_at(left->type), left->ctype, left, NULL, 0);

    return left;
}

static Symbol *get_member(Symbol *ctype, char *name, long *offset) {
    Symbol *member;
    for (member = ctype->first; member; member = member->next) {
        if (member->name) {
            if (!strcmp(member->name, name)) {
                *offset = *offset + member->posn;
                break;
            }
        } else {
            if (member->ctype->name) {
                // a named struct/union, no recursive call
                continue;
            }
            if (member->ctype->ptype == P_STRUCT || member->ctype->ptype == P_UNION) {
                *offset = *offset + member->posn;
                return get_member(member->ctype, name, offset);
            }
        }
    }
    if (!member) {
        fatals("No member found in struct/union", name);
    }

    return member;
}

static ASTnode *access_member(ASTnode *left, int with_pointer) {
    ASTnode *right;
    Symbol *ctype, *member;
    long offset = 0;

    // check variable type
    if (!with_pointer) {
        if (left->type != P_STRUCT && left->type != P_UNION) {
            fatals("Expression is not a struct/union", TEXT);
        } else {
            left->op = A_ADDR;
        }
    }
    if (with_pointer) {
        if (left->type != pointer_to(P_STRUCT) && left->type != pointer_to(P_UNION)) {
            fatals("Expression is not a pointer to struct/union", TEXT);
        }
    }

    left->rvalue = TRUE;
    ctype = left->ctype;

    // skip '.' or '->'
    scan();
    match(T_IDENT, "identifier");

    // get member name and offset
    member = get_member(ctype, TEXT, &offset);

    // get member offset
    right = make_ast_leaf(A_INTLIT, P_LONG, NULL, NULL, offset);
    // add offset to pointer
    left = make_ast_node(A_ADD, pointer_to(member->ptype), member->ctype, left, NULL, right, NULL, 0);
    // dereference pointer to get member value
    left = make_ast_unary(A_DEREF, member->ptype, member->ctype, left, NULL, 0);

    return left;
}

static ASTnode *paren_expr(Symbol **ctype, int ptp) {
    ASTnode *node;
    int type = P_NONE;
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
            type = parse_cast(ctype);
            match(T_RPAREN, ")");
        default:
            node = bin_expr(ptp);
    }

    if (type == P_NONE) {
        match(T_RPAREN, ")");
    } else {
        node = make_ast_unary(A_CAST, type, *ctype, node, NULL, 0);
    }

    return node;
}

static ASTnode *primary(int ptp) {
    ASTnode *node = NULL;
    int label, size, class, type;
    Symbol *ctype, *sym, *var;

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
            return make_ast_leaf(A_INTLIT, P_INT, NULL, NULL, size);
        case T_INTLIT:
            if (TOKEN.int_value >= 0 && TOKEN.int_value <= 0xff) {
                node = make_ast_leaf(A_INTLIT, P_CHAR, NULL, NULL, TOKEN.int_value);
            } else if (TOKEN.int_value <= 0xffff) {
                node = make_ast_leaf(A_INTLIT, P_SHORT, NULL, NULL, TOKEN.int_value);
            } else {
                node = make_ast_leaf(A_INTLIT, P_INT, NULL, NULL, TOKEN.int_value);
            }
            break;
        case T_STRLIT:
            label = gen_new_str(TEXT, TRUE);

            while (peek_token().token_type == T_STRLIT) {
                // get the string literal
                scan();
                gen_new_str(TEXT, FALSE);
            }
            gen_new_str_end();

            node = make_ast_leaf(A_STRLIT, pointer_to(P_CHAR), NULL, NULL, label);
            break;
        case T_IDENT:
            if ((var = find_enum_val_sym(TEXT))) {
                node = make_ast_leaf(A_INTLIT, P_INT, NULL, NULL, var->posn);
                break;
            }

            if (!(var = find_sym(TEXT))) {
                fatals("Unknown variable or function", TEXT);
            }

            switch (var->stype) {
                case S_VARIABLE:
                    node = make_ast_leaf(A_IDENT, var->ptype, var->ctype, var, 0);
                    break;
                case S_FUNCTION:
                    scan();
                    // TODO maybe support function pointer as a parameter?
                    if (TOKEN.token_type != T_LPAREN) {
                        fatal("Missing ( after function name");
                    }
                    return func_call();
                case S_ARRAY:
                    node = make_ast_leaf(A_ADDR, var->ptype, var->ctype, var, 0);
                    node->rvalue = TRUE;
                    break;
                default:
                    fatals("Not a variable or function", TEXT);
            }
            break;
        case T_LPAREN:
            return paren_expr(&ctype, ptp);
        default:
            fatals("syntax error, token", get_name(V_TOKEN, TOKEN.token_type));
    }

    scan();
    return node;
}

static ASTnode *postfix(int ptp) {
    ASTnode *node = primary(ptp);

    while (TRUE) {
        switch (TOKEN.token_type) {
            case T_LBRACKET:
                node = access_array(node);
                break;
            case T_DOT:
                node = access_member(node, FALSE);
                break;
            case T_ARROW:
                node = access_member(node, TRUE);
                break;
            case T_INC:
                if (node->rvalue) {
                    fatal("Can't ++ a rvalue");
                }
                scan();

                if (node->op == A_POSTINC || node->op == A_POSTDEC) {
                    fatal("Can't ++ and/or -- more than once");
                }

                node->op = A_POSTINC;
                break;
            case T_DEC:
                if (node->rvalue) {
                    fatal("Can't ++ a rvalue");
                }
                scan();

                if (node->op == A_POSTINC || node->op == A_POSTDEC) {
                    fatal("Can't ++ and/or -- more than once");
                }

                node->op = A_POSTDEC;
                break;
            default:
                return node;
        }
    }
}

static int token_to_op(int tk) {
    if (tk > T_EOF && tk <= T_MOD) {
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
    if (token_type > T_MOD) {
        fatals("Token with no precendence", get_name(V_TOKEN, token_type));
    }

    int prec = op_prec[token_type];
    if (prec == 0) {
        fatals("Syntax error, token type", get_name(V_TOKEN, token_type));
    }
    return prec;
}

static ASTnode *prefix(int ptp) {
    ASTnode *tree;

    switch (TOKEN.token_type) {
        case T_AND:
            scan();
            tree = prefix(ptp);

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
            tree = prefix(ptp);
            tree->rvalue = TRUE;

            if (!is_ptr(tree->type)) {
                fatal("* operator must be followed by an expression of pointer type");
            }

            tree = make_ast_unary(A_DEREF, value_at(tree->type), tree->ctype, tree, NULL, 0);
            break;
        case T_MINUS:
            scan();
            tree = prefix(ptp);

            tree->rvalue = TRUE;
            if (tree->type == P_CHAR) {
                tree->type = P_INT;
            }
            tree = make_ast_unary(A_NEGATE, tree->type, tree->ctype, tree, NULL, 0);
            break;
        case T_INVERT:
            scan();
            tree = prefix(ptp);

            tree->rvalue = TRUE;
            tree = make_ast_unary(A_INVERT, tree->type, tree->ctype, tree, NULL, 0);
            break;
        case T_LOGNOT:
            scan();
            tree = prefix(ptp);

            tree->rvalue = TRUE;
            tree = make_ast_unary(A_LOGNOT, tree->type, tree->ctype, tree, NULL, 0);
            break;
        case T_INC:
            scan();
            tree = prefix(ptp);

            if (tree->op != A_IDENT) {
                fatal("++ operator must be followed by a variable");
            }
            tree = make_ast_unary(A_PREINC, tree->type, tree->ctype, tree, NULL, 0);
            break;
        case T_DEC:
            scan();
            tree = prefix(ptp);

            if (tree->op != A_IDENT) {
                fatal("-- operator must be followed by a variable");
            }
            tree = make_ast_unary(A_PREDEC, tree->type, tree->ctype, tree, NULL, 0);
            break;
        case T_PLUS:
            scan();
        default:
            tree = postfix(ptp);
    }

    return tree;
}

/*
 * @param ptp: previous token precedence
 */
ASTnode *bin_expr(int ptp) {
    ASTnode *ltemp, *rtemp, *right, *left = prefix(ptp);
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

                right = modify_type(right, left, P_NONE);
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

                return make_ast_node(ast_op, right->type, right->ctype, left, right, ltemp, NULL, 0);
            default:
                left->rvalue = TRUE;
                right->rvalue = TRUE;

                ltemp = modify_type(left, right, ast_op);
                rtemp = modify_type(right, left, ast_op);
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

        left = make_ast_node(ast_op, left->type, left->ctype, left, NULL, right, NULL, 0);

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
