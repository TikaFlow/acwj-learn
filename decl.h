//
// Created by tika on 24-5-19.
//
// Function prototypes for all compiler files

#ifndef ACWJ_LEARN_DEL_H
#define ACWJ_LEARN_DEL_H

#include "defs.h"

// scan.c
int scan(Token *t);

// tree.c
ASTnode *mkastnode(int op, ASTnode *left, ASTnode *right, int intvalue);

ASTnode *mkastleaf(int op, int intvalue);

ASTnode *mkastunary(int op, ASTnode *left, int intvalue);

// expr.c
ASTnode *binexpr(int ptp);

// gen.c
int genAST(ASTnode *node);

void genpreamble();

void genpostamble();

void genfreeregs();

void genprintint(int reg);

// cg.c
void cgfreeregs(void);

void cgpreamble();

void cgpostamble();

int cgload(int value);

int cgadd(int r1, int r2);

int cgsub(int r1, int r2);

int cgmul(int r1, int r2);

int cgdiv(int r1, int r2);

void cgprintint(int reg);

// stmt.c
void statements();

// misc.c
void match(int tokentype, char *what);

void semi();

#endif //ACWJ_LEARN_DEL_H
