//
// Created by tika on 24-5-19.
//
// Function prototypes for all compiler files

#ifndef ACWJ_LEARN_DEL_H
#define ACWJ_LEARN_DEL_H

#include "defs.h"

// scan.c
Token *peek_token();

int scan();

// tree.c
ASTnode *make_ast_node(int op, int type, ASTnode *left, ASTnode *mid, ASTnode *right, int int_value);

ASTnode *make_ast_leaf(int op, int type, int int_value);

ASTnode *make_ast_unary(int op, int type, ASTnode *left, int int_value);

// gen.c
int gen_label();

int gen_ast(ASTnode *node, int reg, int parentASTop);

void gen_pre_amble();

void gen_free_regs();

void gen_print_int(int reg);

void gen_new_sym(int id);

int gen_type_size(int type);

// cg.c
void cg_free_regs();

void cg_pre_amble();

void cg_func_pre_amble(int id);

void cg_func_post_amble(int id);

int cg_load_int(int value);

int cg_load_sym(int id);

int cg_add(int r1, int r2);

int cg_sub(int r1, int r2);

int cg_mul(int r1, int r2);

int cg_div(int r1, int r2);

void cg_print_int(int reg);

int cg_call(int reg, int id);

int cg_store_sym(int r, int id);

void cg_new_sym(int id);

int cg_compare_and_set(int ASTop, int r1, int r2);

int cg_compare_and_jump(int ASTop, int r1, int r2, int label);

void cg_label(int l);

void cg_jump(int l);

int cg_widen(int r, int old_type, int new_type);

int cg_type_size(int type);

void cg_return(int reg, int id);

// expr.c
ASTnode *func_call();

ASTnode *bin_expr(int ptp);

// stmt.c
ASTnode *compound_stmt();

// misc.c
void match(int token_type, char *what);

void fatal(char *s);

void fatals(char *s1, char *s2);

void fatald(char *s, int d);

void fatalc(char *s, int c);

// sym.c
int find_sym(char *s);

int add_sym(char *name, int ptype, int stype, int end_label);

// decl.c
void declare_var();

ASTnode *declare_func();

// types.c
int type_compatible(int *left, int *right, int onlyright);

#endif //ACWJ_LEARN_DEL_H
