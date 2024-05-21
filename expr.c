//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

static ASTnode *primary() {
    ASTnode *node;
    int id;

    switch (TOKEN.token) {
        case T_INTLIT:
            node = mkastleaf(A_INTLIT, TOKEN.intvalue);
            break;
        case T_IDENT:
            if ((id = findglob(TEXT)) < 0) {
                fatals("Unknown variable", TEXT);
            }
            node = mkastleaf(A_IDENT, id);
            break;
        default:
            fatald("syntax error, token", TOKEN.token);
    }

    scan(&TOKEN);
    return node;
}

static int arithop(int tk) {
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
            fatald("Unknown token", TOKEN.token);
    }
    return 0;
}

// TOKEN: EOF + - * / LITERAL
static int op_prec[] = {0, 10, 10, 20, 20, 0};

static int op_precedence(int tokentype) {
    int prec = op_prec[tokentype];
    if (prec == 0) {
        fatald("Syntax error, token type", tokentype);
    }
    return prec;
}

/*
 * @param ptp: previous token precedence
 */
ASTnode *binexpr(int ptp) {
    ASTnode *right, *left = primary();
    int tokentype = TOKEN.token;

    if (tokentype == T_SEMI) {
        return left;
    }

    while (op_precedence(tokentype) > ptp) {
        scan(&TOKEN);

        right = binexpr(op_prec[tokentype]);

        left = mkastnode(arithop(tokentype), left, right, 0);

        tokentype = TOKEN.token;
        if (tokentype == T_SEMI)
            return left;
    }

    return left;
}
