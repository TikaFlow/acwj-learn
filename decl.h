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
ASTnode *mkastnode(int op, ASTnode *left,ASTnode *mid, ASTnode *right, int intvalue);

ASTnode *mkastleaf(int op, int intvalue);

ASTnode *mkastunary(int op, ASTnode *left, int intvalue);

// gen.c
int genAST(ASTnode *node, int reg, int parentASTop);

void genpreamble();

void genpostamble();

void genfreeregs();

void genprintint(int reg);

void genglobsym(char *s);

// cg.c
void cgfreeregs(void);

void cgpreamble();

void cgpostamble();

int cgloadint(int value);

int cgloadglob(char *identifier);

int cgadd(int r1, int r2);

int cgsub(int r1, int r2);

int cgmul(int r1, int r2);

int cgdiv(int r1, int r2);

void cgprintint(int reg);

int cgstorglob(int r, char *identifier);

void cgglobsym(char *sym);

int cgcompare_and_set(int ASTop, int r1, int r2);

int cgcompare_and_jump(int ASTop, int r1, int r2, int label);

void cglabel(int l);

void cgjump(int l);

// expr.c
ASTnode *binexpr(int ptp);

// stmt.c
ASTnode *compoundstmt();

// misc.c
void match(int tokentype, char *what);

void fatal(char *s);

void fatals(char *s1, char *s2);

void fatald(char *s, int d);

void fatalc(char *s, int c);

// sym.c
int findglob(char *s);

int addglob(char *name);

// decl.c
void declarevar(void);

#endif //ACWJ_LEARN_DEL_H
