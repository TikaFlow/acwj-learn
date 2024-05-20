//
// Created by tika on 24-5-19.
//
// Function prototypes for all compiler files

#ifndef ACWJ_LEARN_DEL_H
#define ACWJ_LEARN_DEL_H

#include "defs.h"

int scan(Token *t);

ASTnode *mkastnode(int op, ASTnode *left, ASTnode *right, int intvalue);

ASTnode *mkastleaf(int op, int intvalue);

ASTnode *mkastunary(int op, ASTnode *left, int intvalue);

ASTnode *binexpr(void);

int interpretAST(ASTnode *n);

#endif //ACWJ_LEARN_DEL_H
