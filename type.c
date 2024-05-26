//
// Created by tika on 24-5-24.
//

#include "data.h"
#include "decl.h"

static int is_int(int type) {
    if (type >= P_CHAR && type <= P_LONG) {
        return TRUE;
    }
    return FALSE;
}

static int is_ptr(int type) {
    if (type >= P_VOIDPTR && type <= P_LONGPTR) {
        return TRUE;
    }
    return FALSE;
}

int type_compatible(int *left, int *right, int onlyright) {

    if (*left == *right) {
        *left = *right = A_NONE;
        return TRUE;
    }

    int left_size = gen_type_size(*left), right_size = gen_type_size(*right);

    if (left_size == 0 || right_size == 0) {
        return FALSE;
    }

    if (left_size < right_size) {
        *left = A_WIDEN;
        *right = A_NONE;

        return TRUE;
    }

    if (left_size > right_size) {
        if (onlyright) {
            return FALSE;
        }

        *left = A_NONE;
        *right = A_WIDEN;

        return TRUE;
    }

    *left = *right = A_NONE;
    return TRUE;
}

int pointer_to(int type) {
    int new_type;

    switch (type) {
        case P_VOID:
            new_type = P_VOIDPTR;
            break;
        case P_CHAR:
            new_type = P_CHARPTR;
            break;
        case P_INT:
            new_type = P_INTPTR;
            break;
        case P_LONG:
            new_type = P_LONGPTR;
            break;
        default:
            new_type = P_NONE;
            fatald("Unrecognized in pointer_to: unknown type", type);
    }

    return new_type;
}

int value_at(int type) {
    int new_type;

    switch (type) {
        case P_VOIDPTR:
            new_type = P_VOID;
            break;
        case P_CHARPTR:
            new_type = P_CHAR;
            break;
        case P_INTPTR:
            new_type = P_INT;
            break;
        case P_LONGPTR:
            new_type = P_LONG;
            break;
        default:
            new_type = P_NONE;
            fatald("Unrecognized in value_at: unknown type", type);
    }

    return new_type;
}

ASTnode *modify_type(ASTnode *tree, int rtype, int op) {
    int lsize, rsize, ltype = tree->type;

    if (is_int(ltype) && is_int(rtype)) {
        if (ltype == rtype) {
            return tree;
        }

        lsize = gen_type_size(ltype);
        rsize = gen_type_size(rtype);

        if (lsize > rsize) {
            return NULL;
        }

        if (lsize < rsize) {
            return make_ast_unary(A_WIDEN, rtype, tree, 0);
        }
    }

    if (is_ptr(ltype)) {
        // I don't know why op must be zero,
        // but it works when I remove it
        // if (!op && ltype == rtype) {
        if (ltype == rtype) {
            return tree;
        }
    }

    if (op == A_ADD || op == A_SUBTRACT) {
        if (is_int(ltype) && is_ptr(rtype)) {
            rsize = gen_type_size(value_at(rtype));
            if (rsize > 1) {
                return make_ast_unary(A_SCALE, rtype, tree, rsize);
            }
        }
    }

    return NULL;
}
