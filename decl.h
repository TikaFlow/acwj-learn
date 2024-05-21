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

ASTnode *binexpr(int ptp);

int interpretAST(ASTnode *n);

void generatecode(ASTnode *n);

void freeall_registers(void);

void cgpreamble();

void cgpostamble();

int cgload(int value);

int cgadd(int r1, int r2);

int cgsub(int r1, int r2);

int cgmul(int r1, int r2);

int cgdiv(int r1, int r2);

void cgprintint(int r);

#endif //ACWJ_LEARN_DEL_H
