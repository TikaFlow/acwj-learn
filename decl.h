//
// Created by tika on 24-5-19.
//
// Function prototypes for all compiler files

#ifndef ACWJ_LEARN_DEL_H
#define ACWJ_LEARN_DEL_H

#include "defs.h"

// scan.c
Token peek_token();

int scan();

// tree.c
ASTnode *make_ast_node(int op, int type, Symbol *ctype, ASTnode *left, ASTnode *mid, ASTnode *right, Symbol *sym,
                       long int_value);

ASTnode *make_ast_leaf(int op, int type, Symbol *ctype, Symbol *sym, long int_value);

ASTnode *make_ast_unary(int op, int type, Symbol *ctype, ASTnode *left, Symbol *sym, long int_value);

// gen.c
int gen_label();

int gen_ast(ASTnode *node, int if_label, int start_label, int end_label, int parent_op);

void gen_pre_amble(char *file);

void gen_post_amble();

void gen_free_regs(int keep_reg);

void gen_new_sym(Symbol *sym);

int gen_new_str(char *str, int first);

void gen_new_str_end();

int gen_type_size(int type);

int gen_align(int type, int offset, int direction);

// cg.c
void cg_free_regs(int keep_reg);

int cg_alloc_register();

void cg_free_register(int reg);

void cg_pre_amble(char *file);

void cg_post_amble();

void cg_func_pre_amble(Symbol *sym);

void cg_func_post_amble(Symbol *sym);

int cg_load_int(long value);

int cg_load_global_sym(Symbol *sym, int op);

int cg_load_local_sym(Symbol *sym, int op);

int cg_load_str(int label);

int cg_add(int r1, int r2);

int cg_sub(int r1, int r2);

int cg_mul(int r1, int r2);

int cg_div(int r1, int r2);

int cg_mod(int r1, int r2);

int cg_sal_n(int reg, int n);

int cg_call(Symbol *sym, int args_num);

void cg_copy_arg(int reg, int arg_pos);

int cg_store_global_sym(int r, Symbol *sym);

int cg_store_local_sym(int r, Symbol *sym);

void cg_new_sym(Symbol *sym);

void cg_new_str(int label, char *str);

void cg_new_str_end();

int cg_compare_and_set(ASTnode *node, int r1, int r2);

int cg_compare_and_jump(ASTnode *node, int r1, int r2, int label);

void cg_label(int l);

int cg_jump(int l);

int cg_widen(int r, int old_type, int new_type);

int cg_type_size(int type);

int cg_return(int reg, Symbol *sym);

int cg_address(Symbol *sym);

int cg_deref(int reg, int type);

int cg_store_deref(int r1, int r2, int type);

int cg_negate(int r);

int cg_invert(int r);

int cg_lognot(int r);

int cg_tobool(int r, int op, int label);

int cg_load_bool(int reg, int val);

int cg_and(int r1, int r2);

int cg_or(int r1, int r2);

int cg_xor(int r1, int r2);

int cg_sal(int r1, int r2);

int cg_sar(int r1, int r2);

int cg_align(int type, int offset, int direction);

void cg_switch(int reg, int case_cnt, int *case_label, int *case_val, int dft_label);

void cg_mov_reg(int r1, int r2);

void cg_line_num(int line_num);

// expr.c
ASTnode *expression_list(int end_token);

ASTnode *func_call();

ASTnode *bin_expr(int ptp);

// stmt.c
ASTnode *compound_stmt(int is_switch, Symbol **head, Symbol **tail);

// misc.c
void match(int token_type, char *what);

char *get_name(int value_type, int value);

void debug(char *s);

void warning(char *s);

void fatal(char *s);

void fatals(char *s1, char *s2);

void fatald(char *s, int d);

void fatalc(char *s, int c);

char *format_str(char *fmt, ...);

// sym.c
void reset_global_syms();

void reset_func_ptr();

void set_func_ptr(Symbol *func);

void reset_static_syms();

Symbol *find_global_sym(char *s);

Symbol *find_param_local_sym(char *s);

Symbol *find_struct_sym(char *s);

Symbol *find_union_sym(char *s);

Symbol *find_member_sym(char *s, Symbol *head);

Symbol *find_enum_type_sym(char *s);

Symbol *find_enum_val_sym(char *s);

Symbol *find_typedef_sym(char *s);

Symbol *find_sym(char *s);

Symbol *add_global_sym(char *name, int ptype, Symbol *ctype, int stype, int class, int n_elem, int posn);

Symbol *add_local_sym(char *name, Symbol **head, Symbol **tail, int ptype, Symbol *ctype, int stype, int n_elem);

Symbol *add_param_sym(char *name, Symbol **head, Symbol **tail, int ptype, Symbol *ctype, int stype);

Symbol *add_struct_sym(char *name);

Symbol *add_union_sym(char *name);

Symbol *add_member_sym(char *name, Symbol **head, Symbol **tail, int ptype, Symbol *ctype, int stype, int n_elem);

Symbol *add_enum_sym(char *name, int class, int value);

Symbol *add_typedef_sym(char *name, int ptype, Symbol *ctype);

void dump_sym_tables();

// decl.c
int
declare_list(Symbol **ctype, int class, int end_tk1, int end_tk2, ASTnode **glue_tree, Symbol **head, Symbol **tail);

void declare_global();

int parse_type(Symbol **ctype, int *class);

int parse_stars(int type);

int parse_cast(Symbol **ctype);

// type.c
int is_int(int type);

int is_ptr(int type);

int pointer_to(int type);

int value_at(int type);

ASTnode *modify_type(ASTnode *left, ASTnode *right, int op);

int size_of_type(int type, Symbol *ctype);

// opt.c
ASTnode *optimize(ASTnode *tree);

#endif //ACWJ_LEARN_DEL_H
