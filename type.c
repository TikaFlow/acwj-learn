//
// Created by tika on 24-5-24.
//

#include "data.h"
#include "decl.h"

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
