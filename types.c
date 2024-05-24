//
// Created by tika on 24-5-24.
//

#include "data.h"
#include "decl.h"

int type_compatible(int *left, int *right, int onlyright) {
    if (*left == P_VOID || *right == P_VOID) {
        return FALSE;
    }

    if (*left == *right) {
        *left = *right = A_NONE;
        return TRUE;
    }

    // widen char to int
    if (*left == P_CHAR && *right == P_INT) {
        *left = A_WIDEN;
        *right = A_NONE;
        return TRUE;
    }
    if (*left == P_INT && *right == P_CHAR) {
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
