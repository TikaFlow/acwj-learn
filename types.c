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
