//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void declarevar() {
    match(T_INT, "int");
    match(T_IDENT, "identifier");
    addglob(TEXT);
    genglobsym(TEXT);
    match(T_SEMI, ";");
}

ASTnode *declarefunc() {
    ASTnode *tree;
    int nameslot;

    match(T_VOID, "void");
    match(T_IDENT, "identifier");
    nameslot = addglob(TEXT);
    match(T_LPAREN, "(");
    match(T_RPAREN, ")");

    tree = compoundstmt();

    return mkastunary(A_FUNCTION, tree, nameslot);
}
