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

ASTnode *modify_type(ASTnode *left, ASTnode *right, int op) {
    int lsize, rsize, ltype = left->type, rtype = right->type;

    if (op == A_LOGAND || op == A_LOGOR) {
        if (ltype == P_STRUCT || ltype == P_UNION || rtype == P_STRUCT || rtype == P_UNION) {
            return NULL;
        }
        return left;
    }

    if (ltype == P_STRUCT || ltype == P_UNION || rtype == P_STRUCT || rtype == P_UNION) {
        fatal("I don't know how to deal with it yet");
    }

    if (is_int(ltype) && is_int(rtype)) {
        if (ltype == rtype) {
            return left;
        }

        lsize = size_of_type(ltype, NULL);
        rsize = size_of_type(rtype, NULL);

        if (lsize > rsize) {
            return NULL;
        }

        if (lsize < rsize) {
            return make_ast_unary(A_WIDEN, rtype, NULL, left, NULL, 0);
        }
    }

    if (is_ptr(ltype) && is_ptr(rtype)) {
        if (op >= A_EQ && op <= A_GE) {
            return left;
        }

        if (op == P_NONE && (ltype == rtype || ltype == pointer_to(P_VOID))) {
            return left;
        }
    }

    if (op == A_ADD || op == A_SUBTRACT || op == A_ASPLUS || op == A_ASMINUS) {
        if (is_int(ltype) && is_ptr(rtype)) {
            rsize = gen_type_size(value_at(rtype));
            if (rsize > 1) {
                return make_ast_unary(A_SCALE, rtype, right->ctype, left, NULL, rsize);
            }
            return left;
        }
    }

    return NULL;
}
