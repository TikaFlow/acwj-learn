//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

static ASTnode *primary(void) {
    struct ASTnode *n;

    switch (TOKEN.token) {
        case T_INTLIT:
            n = mkastleaf(A_INTLIT, TOKEN.intvalue);
            scan(&TOKEN);
            return (n);
        default:
            fprintf(stderr, "syntax error on line %d\n", LINE);
            exit(1);
    }
}

int arithop(int tk) {
    switch (tk) {
        case T_PLUS:
            return A_ADD;
        case T_MINUS:
            return A_SUBTRACT;
        case T_STAR:
            return A_MULTIPLY;
        case T_SLASH:
            return A_DIVIDE;
        default:
            fprintf(stderr, "unknown token in arithop() on line %d\n", LINE);
            exit(1);
    }
}

ASTnode *binexpr(void) {
    ASTnode *node, *left, *right;
    int nodetype;

    left = primary();
    if (TOKEN.token == T_EOF) {
        return left;
    }
    nodetype = arithop(TOKEN.token);
    scan(&TOKEN);
    right = binexpr();
    node = mkastnode(nodetype, left, right, 0);
    return node;
}
