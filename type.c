//
// Created by tika on 24-5-24.
//

#include "data.h"
#include "decl.h"

int is_int(int type) {
    return !(type & 0xf) && type >= P_CHAR && type <= P_LONG;
}

int is_ptr(int type) {
    return type & 0xf;
}

int pointer_to(int type) {
    if ((type & 0xf) == 0xf) {
        fatals("The pointer is too deep at pointer_to(), type", get_name(V_PTYPE, type));
    }

    return type + 1;
}

int value_at(int type) {
    if (!(type & 0xf)) {
        fatals("Unable to dereference a non-pointer at value_at(), type", get_name(V_PTYPE, type));
    }

    return type - 1;
}

int size_of_type(int ptype, Symbol *ctype) {
    return (ptype == P_STRUCT || ptype == P_UNION) ? ctype->size : gen_type_size(ptype);
}

ASTnode *modify_type(ASTnode *tree, int rtype, int op) {
    int lsize, rsize, ltype = tree->type;

    if (ltype == P_STRUCT || ltype == P_UNION || rtype == P_STRUCT || rtype == P_UNION) {
        fatal("I don't know how to deal with it yet");
    }

    if (is_int(ltype) && is_int(rtype)) {
        if (ltype == rtype) {
            return tree;
        }

        lsize = size_of_type(ltype, NULL);
        rsize = size_of_type(rtype, NULL);

        if (lsize > rsize) {
            return NULL;
        }

        if (lsize < rsize) {
            return make_ast_unary(A_WIDEN, rtype, tree, NULL, 0);
        }
    }

    if (is_ptr(ltype) && is_ptr(rtype)) {
        if (op == P_NONE && (ltype == rtype || ltype == pointer_to(P_VOID))) {
            return tree;
        }
    }

    if (op == A_ADD || op == A_SUBTRACT) {
        if (is_int(ltype) && is_ptr(rtype)) {
            rsize = gen_type_size(value_at(rtype));
            if (rsize > 1) {
                return make_ast_unary(A_SCALE, rtype, tree, NULL, rsize);
            }
            return tree;
        }
    }

    return NULL;
}
