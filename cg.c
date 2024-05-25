//
// Created by tika on 24-5-21.
//
// generate AT&T assembly code

#include "data.h"
#include "decl.h"

static int freereg[4];
static char *reglist[] = {"%r8", "%r9", "%r10", "%r11"};
static char *breglist[] = {"%r8b", "%r9b", "%r10b", "%r11b"};
static char *dreglist[] = {"%r8d", "%r9d", "%r10d", "%r11d"};
static char *cmplist[] = {"sete", "setne", "setl", "setg", "setle", "setge"};
static char *invcmplist[] = {"jne", "je", "jge", "jle", "jg", "jl"};
static int type_size[] = {
        0, // P_NONE
        0, // P_VOID
        1, // P_CHAR
        4, // P_INT
        8, // P_LONG
        8, // P_VOIDPTR
        8, // P_CHARPTR
        8, // P_INTPTR
        8 // P_LONGPTR
};

void cg_free_regs() {
    for (int i = 0; i < 4; i++) {
        freereg[i] = 1;
    }
}

static int alloc_register() {
    for (int i = 0; i < 4; i++) {
        if (freereg[i]) {
            freereg[i] = FALSE;
            return i;
        }
    }
    fprintf(stderr, "Out of registers!\n");
    exit(1);
}

static void free_register(int reg) {
    if (freereg[reg]) {
        fprintf(stderr, "Error trying to free register %d\n", reg);
        exit(1);
    }
    freereg[reg] = TRUE;
}

void cg_pre_amble() {
    cg_free_regs();
    fputs("\t.text\n", OUT_FILE);
}

void cg_func_pre_amble(int id) {
    char *name = SYM_TAB[id].name;
    fprintf(OUT_FILE,
            "\t.text\n"
            "\t.globl\t%s\n"
            "\t.type\t%s, @function\n"
            "%s:\n"
            "\tpushq\t%%rbp\n"
            "\tmovq\t%%rsp, %%rbp\n",
            name, name, name);
}

void cg_func_post_amble(int id) {
    cg_label(SYM_TAB[id].end_label);
    fputs("\tpopq\t%rbp\n"
          "\tret\n",
          OUT_FILE);
}

int cg_load_int(long value) {
    int reg = alloc_register();

    fprintf(OUT_FILE, "\tmovq\t$%ld, %s\n", value, reglist[reg]);
    return reg;
}

int cg_load_sym(int id) {
    int reg = alloc_register();
    switch (SYM_TAB[id].ptype) {
        case P_CHAR:
            fprintf(OUT_FILE, "\tmovzbq\t%s(%%rip), %s\n", SYM_TAB[id].name, reglist[reg]);
            break;
        case P_INT:
            fprintf(OUT_FILE, "\tmovslq\t%s(%%rip), %s\n", SYM_TAB[id].name, reglist[reg]);
            break;
        case P_LONG:
        case P_CHARPTR:
        case P_INTPTR:
        case P_LONGPTR:
            fprintf(OUT_FILE, "\tmovq\t%s(%%rip), %s\n", SYM_TAB[id].name, reglist[reg]);
            break;
        default:
            fatald("Bad type in cg_load_sym()", SYM_TAB[id].ptype);
    }
    return reg;
}

int cg_add(int r1, int r2) {
    fprintf(OUT_FILE, "\taddq\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_sub(int r1, int r2) {
    fprintf(OUT_FILE, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_mul(int r1, int r2) {
    fprintf(OUT_FILE, "\timulq\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_div(int r1, int r2) {
    fprintf(OUT_FILE, "\tmovq\t%s,%%rax\n", reglist[r1]);
    fprintf(OUT_FILE, "\tcqto\n");
    fprintf(OUT_FILE, "\tidivq\t%s\n", reglist[r2]);
    fprintf(OUT_FILE, "\tmovq\t%%rax, %s\n", reglist[r1]);
    free_register(r2);
    return r1;
}

void cg_print_int(int reg) {
    fprintf(OUT_FILE, "\tmovq\t%s,%%rdi\n", reglist[reg]);
    fprintf(OUT_FILE, "\tcall\tprint_int\n");
    free_register(reg);
}

int cg_call(int reg, int id) {
    int out_reg = alloc_register();

    fprintf(OUT_FILE, "\tmovq\t%s, %%rdi\n", reglist[reg]);
    fprintf(OUT_FILE, "\tcall\t%s\n", SYM_TAB[id].name);
    fprintf(OUT_FILE, "\tmovq\t%%rax, %s\n", reglist[out_reg]);

    free_register(reg);
    return out_reg;
}

int cg_store_sym(int reg, int id) {
    switch (SYM_TAB[id].ptype) {
        case P_CHAR:
            fprintf(OUT_FILE, "\tmovb\t%s, %s(%%rip)\n", breglist[reg], SYM_TAB[id].name);
            break;
        case P_INT:
            fprintf(OUT_FILE, "\tmovl\t%s, %s(%%rip)\n", dreglist[reg], SYM_TAB[id].name);
            break;
        case P_LONG:
        case P_CHARPTR:
        case P_INTPTR:
        case P_LONGPTR:
            fprintf(OUT_FILE, "\tmovq\t%s, %s(%%rip)\n", reglist[reg], SYM_TAB[id].name);
            break;
        default:
            fatald("Bad type in cg_store_sym()", SYM_TAB[id].ptype);
    }
    return reg;
}

int cg_type_size(int type) {
    if (type < P_NONE || type > P_LONGPTR) {
        fatal("Bad type in gen_type_size()");
    }
    return type_size[type];
}

void cg_new_sym(int id) {
    int size = cg_type_size(SYM_TAB[id].ptype);
    fprintf(OUT_FILE, "\t.comm\t%s, %d, %d\n", SYM_TAB[id].name, size, size);
}

void cg_label(int l) {
    fprintf(OUT_FILE, "L%d:\n", l);
}

void cg_jump(int l) {
    fprintf(OUT_FILE, "\tjmp\tL%d\n", l);
}

int cg_compare_and_set(int ASTop, int r1, int r2) {
    if (ASTop < A_EQ || ASTop > A_GE) {
        fatal("Bad ASTop in cg_compare_and_set()");
    }

    fprintf(OUT_FILE, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
    fprintf(OUT_FILE, "\t%s\t%s\n", cmplist[ASTop - A_EQ], breglist[r2]);
    fprintf(OUT_FILE, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);
    free_register(r1);

    return r2;
}

int cg_compare_and_jump(int ASTop, int r1, int r2, int l) {
    if (ASTop < A_EQ || ASTop > A_GE) {
        fatal("Bad ASTop in cg_compare_and_jump()");
    }

    fprintf(OUT_FILE, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
    fprintf(OUT_FILE, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], l);
    cg_free_regs();

    return NO_REG;
}

int cg_widen(int r, int old_type, int new_type) {
    return r;
}

void cg_return(int reg, int id) {
    switch (SYM_TAB[id].ptype) {
        case P_CHAR:
            fprintf(OUT_FILE, "\tmovzbq\t%s, %%rax\n", breglist[reg]);
            break;
        case P_INT:
            fprintf(OUT_FILE, "\tmovslq\t%s, %%rax\n", dreglist[reg]);
            break;
        case P_LONG:
            fprintf(OUT_FILE, "\tmovq\t%s, %%rax\n", reglist[reg]);
            break;
        default:
            fatald("Bad function type in cg_return()", SYM_TAB[id].ptype);
    }
    cg_jump(SYM_TAB[id].end_label);
}

int cg_address(int id) {
    int reg = alloc_register();

    fprintf(OUT_FILE,
            "\tleaq\t%s(%%rip), %s\n",
            SYM_TAB[id].name,
            reglist[reg]);

    return reg;
}

int cg_deref(int reg, int type) {
    fprintf(OUT_FILE, "\tmovq\t(%s), %s\n", reglist[reg], reglist[reg]);

    return reg;
}
