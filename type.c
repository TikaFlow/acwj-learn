//
// Created by tika on 24-5-24.
//

#include "data.h"
#include "decl.h"

int is_int(int type) {
    return !(type & 0xf);
}

int is_ptr(int type) {
    return type & 0xf;
}

int pointer_to(int type) {
    if ((type & 0xf) == 0xf) {
        fatald("The pointer is too deep at pointer_to(), type", type);
    }

    return type + 1;
}

int value_at(int type) {
    if (!(type & 0xf)) {
        fatald("Unable to dereference a non-pointer at value_at(), type", type);
    }

    return type - 1;
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
        if (!op && ltype == rtype) {
            return tree;
        }
    }

    if (op == A_ADD || op == A_SUBTRACT) {
        if (is_int(ltype) && is_ptr(rtype)) {
            rsize = gen_type_size(value_at(rtype));
            if (rsize > 1) {
                return make_ast_unary(A_SCALE, rtype, tree, rsize);
            }
            return tree;
        }
    }

    return NULL;
}
