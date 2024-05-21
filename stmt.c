//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void statements() {
    ASTnode *tree;
    int reg;

    while (1) {
        match(T_PRINT, "print");
        tree = binexpr(0);
        reg = genAST(tree);
        genprintint(reg);
        genfreeregs();
        semi();

        if (TOKEN.token == T_EOF) {
            return;
        }
    }
}
