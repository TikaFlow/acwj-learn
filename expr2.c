//
// Created by tika on 24-5-20.
//

#include "data.h"
#include "decl.h"

static ASTnode *primary() {
    ASTnode *n;

    // For an INTLIT token, make a leaf AST node for it
    // and scan in the next token. Otherwise, a syntax error
    // for any other token type.
    switch (TOKEN.token) {
        case T_INTLIT:
            n = mkastleaf(A_INTLIT, TOKEN.intvalue);
            scan(&TOKEN);
            return (n);
        default:
            fprintf(stderr, "syntax error on line %d, token %d\n", LINE, TOKEN.token);
            exit(1);
    }
}


// Convert a binary operator token into an AST operation.
static int arithop(int tok) {
    switch (tok) {
        case T_PLUS:
            return (A_ADD);
        case T_MINUS:
            return (A_SUBTRACT);
        case T_STAR:
            return (A_MULTIPLY);
        case T_SLASH:
            return (A_DIVIDE);
        default:
            fprintf(stderr, "syntax error on line %d, token %d\n", LINE, tok);
            exit(1);
    }
}

ASTnode *additive_expr();

// Return an AST tree whose root is a '*' or '/' binary operator
ASTnode *multiplicative_expr() {
    ASTnode *left, *right;
    int tokentype;

    // Get the integer literal on the left.
    // Fetch the next token at the same time.
    left = primary();

    // If no tokens left, return just the left node
    tokentype = TOKEN.token;
    if (tokentype == T_EOF)
        return (left);

    // While the token is a '*' or '/'
    while ((tokentype == T_STAR) || (tokentype == T_SLASH)) {
        // Fetch in the next integer literal
        scan(&TOKEN);
        right = primary();

        // Join that with the left integer literal
        left = mkastnode(arithop(tokentype), left, right, 0);

        // Update the details of the current token.
        // If no tokens left, return just the left node
        tokentype = TOKEN.token;
        if (tokentype == T_EOF)
            break;
    }

    // Return whatever tree we have created
    return (left);
}

// Return an AST tree whose root is a '+' or '-' binary operator
ASTnode *additive_expr() {
    ASTnode *left, *right;
    int tokentype;

    // Get the left sub-tree at a higher precedence than us
    left = multiplicative_expr();

    // If no tokens left, return just the left node
    tokentype = TOKEN.token;
    if (tokentype == T_EOF)
        return (left);

    // Cache the '+' or '-' token type

    // Loop working on token at our level of precedence
    while (1) {
        // Fetch in the next integer literal
        scan(&TOKEN);

        // Get the right sub-tree at a higher precedence than us
        right = multiplicative_expr();

        // Join the two sub-trees with our low-precedence operator
        left = mkastnode(arithop(tokentype), left, right, 0);

        // And get the next token at our precedence
        tokentype = TOKEN.token;
        if (tokentype == T_EOF)
            break;
    }

    // Return whatever tree we have created
    return (left);
}

ASTnode *binexpr(int n) {
    return (additive_expr());
}
