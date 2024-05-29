//
// Created by tika on 24-5-21.
//
// generate AT&T assembly code

#include "data.h"
#include "decl.h"

#define FREE_REG_NUM 4

enum {
    NO_SECTION,
    TEXT_SECTION,
    DATA_SECTION
} cur_section = NO_SECTION;

static int local_offset, stack_offset;
static int freereg[FREE_REG_NUM];
static char *breglist[] = {"%r10b", "%r11b", "%r12b", "%r13b", "%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *dreglist[] = {"%r10d", "%r11d", "%r12d", "%r13d", "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
static char *reglist[] = {"%r10", "%r11", "%r12", "%r13", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
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

void cg_text_section() {
    if (cur_section != TEXT_SECTION) {
        fprintf(OUT_FILE, "\t.text\n");
        cur_section = TEXT_SECTION;
    }
}

void cg_data_section() {
    if (cur_section != DATA_SECTION) {
        fprintf(OUT_FILE, "\t.data\n");
        cur_section = DATA_SECTION;
    }
}

static void cg_reset_local_offset() {
    local_offset = 0;
}

static int cg_get_local_offset(int type) {
    int size = cg_type_size(type);
    local_offset += size > 4 ? size : 4;

    return -local_offset;
}

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
}

// Nothing to do
void cg_post_amble() {
}

void cg_func_pre_amble(int id) {
    char *name = SYM_TAB[id].name;
    int i, param_offset = 0x10, param_reg = FREE_REG_NUM;
    cg_text_section();
    cg_reset_local_offset();

    fprintf(OUT_FILE,
            "\t.globl\t%s\n"
            "\t.type\t%s, @function\n"
            "%s:\n"
            "\tpushq\t%%rbp\n"
            "\tmovq\t%%rsp, %%rbp\n",
            name, name, name);

    for (i = SYM_TAB_LEN - 1; i > LOCAL_TOP; i--) {
        if (SYM_TAB[i].class != C_PARAM) {
            break;
        }
        if (i < SYM_TAB_LEN - 6) {
            break;
        }

        SYM_TAB[i].posn = cg_get_local_offset(SYM_TAB[i].ptype);
        cg_store_local_sym(param_reg++, i);
    }

    for (; i > LOCAL_TOP; i--) {
        if (SYM_TAB[i].class == C_PARAM) {
            SYM_TAB[i].posn = param_offset;
            param_offset += 8;
        } else {
            SYM_TAB[i].posn = cg_get_local_offset(SYM_TAB[i].ptype);
        }
    }

    stack_offset = (local_offset + 0xF) & ~0xF;
    fprintf(OUT_FILE, "\tsubq\t$%d, %%rsp\n", stack_offset);
}

void cg_func_post_amble(int id) {
    cg_label(SYM_TAB[id].end_label);
    fprintf(OUT_FILE,
            "\taddq\t$%d, %%rsp\n"
            "\tpopq\t%%rbp\n"
            "\tret\n",
            stack_offset);
}

int cg_load_int(long value) {
    int reg = alloc_register();

    fprintf(OUT_FILE, "\tmovq\t$%ld, %s\n", value, reglist[reg]);
    return reg;
}

int cg_load_global_sym(int id, int op) {
    int reg = alloc_register();
    switch (SYM_TAB[id].ptype) {
        case P_CHAR:
            if (op == A_PREINC) {
                fprintf(OUT_FILE, "\tincb\t%s\n", SYM_TAB[id].name);
            }
            if (op == A_PREDEC) {
                fprintf(OUT_FILE, "\tdecb\t%s\n", SYM_TAB[id].name);
            }
            fprintf(OUT_FILE, "\tmovzbq\t%s(%%rip), %s\n", SYM_TAB[id].name, reglist[reg]);
            if (op == A_POSTINC) {
                fprintf(OUT_FILE, "\tincb\t%s\n", SYM_TAB[id].name);
            }
            if (op == A_POSTDEC) {
                fprintf(OUT_FILE, "\tdecb\t%s\n", SYM_TAB[id].name);
            }
            break;
        case P_INT:
            if (op == A_PREINC) {
                fprintf(OUT_FILE, "\tincl\t%s\n", SYM_TAB[id].name);
            }
            if (op == A_PREDEC) {
                fprintf(OUT_FILE, "\tdecl\t%s\n", SYM_TAB[id].name);
            }
            fprintf(OUT_FILE, "\tmovslq\t%s(%%rip), %s\n", SYM_TAB[id].name, reglist[reg]);
            if (op == A_POSTINC) {
                fprintf(OUT_FILE, "\tincl\t%s\n", SYM_TAB[id].name);
            }
            if (op == A_POSTDEC) {
                fprintf(OUT_FILE, "\tdecl\t%s\n", SYM_TAB[id].name);
            }
            break;
        case P_LONG:
        case P_CHARPTR:
        case P_INTPTR:
        case P_LONGPTR:
            if (op == A_PREINC) {
                fprintf(OUT_FILE, "\tincq\t%s\n", SYM_TAB[id].name);
            }
            if (op == A_PREDEC) {
                fprintf(OUT_FILE, "\tdecq\t%s\n", SYM_TAB[id].name);
            }
            fprintf(OUT_FILE, "\tmovq\t%s(%%rip), %s\n", SYM_TAB[id].name, reglist[reg]);
            if (op == A_POSTINC) {
                fprintf(OUT_FILE, "\tincq\t%s\n", SYM_TAB[id].name);
            }
            if (op == A_POSTDEC) {
                fprintf(OUT_FILE, "\tdecq\t%s\n", SYM_TAB[id].name);
            }
            break;
        default:
            fatald("Bad type in cg_load_global_sym()", SYM_TAB[id].ptype);
    }
    return reg;
}

int cg_load_local_sym(int id, int op) {
    int reg = alloc_register();
    switch (SYM_TAB[id].ptype) {
        case P_CHAR:
            if (op == A_PREINC) {
                fprintf(OUT_FILE, "\tincb\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            if (op == A_PREDEC) {
                fprintf(OUT_FILE, "\tdecb\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            fprintf(OUT_FILE, "\tmovzbq\t%d(%%rbp), %s\n", SYM_TAB[id].posn, reglist[reg]);
            if (op == A_POSTINC) {
                fprintf(OUT_FILE, "\tincb\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            if (op == A_POSTDEC) {
                fprintf(OUT_FILE, "\tdecb\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            break;
        case P_INT:
            if (op == A_PREINC) {
                fprintf(OUT_FILE, "\tincl\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            if (op == A_PREDEC) {
                fprintf(OUT_FILE, "\tdecl\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            fprintf(OUT_FILE, "\tmovslq\t%d(%%rbp), %s\n", SYM_TAB[id].posn, reglist[reg]);
            if (op == A_POSTINC) {
                fprintf(OUT_FILE, "\tincl\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            if (op == A_POSTDEC) {
                fprintf(OUT_FILE, "\tdecl\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            break;
        case P_LONG:
        case P_CHARPTR:
        case P_INTPTR:
        case P_LONGPTR:
            if (op == A_PREINC) {
                fprintf(OUT_FILE, "\tincq\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            if (op == A_PREDEC) {
                fprintf(OUT_FILE, "\tdecq\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            fprintf(OUT_FILE, "\tmovq\t%d(%%rbp), %s\n", SYM_TAB[id].posn, reglist[reg]);
            if (op == A_POSTINC) {
                fprintf(OUT_FILE, "\tincq\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            if (op == A_POSTDEC) {
                fprintf(OUT_FILE, "\tdecq\t%d(%%rbp)\n", SYM_TAB[id].posn);
            }
            break;
        default:
            fatald("Bad type in cg_load_local_sym()", SYM_TAB[id].ptype);
    }
    return reg;
}

int cg_load_str(int id) {
    int reg = alloc_register();
    fprintf(OUT_FILE, "\tleaq\tL%d(%%rip), %s\n", id, reglist[reg]);
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

int cg_negate(int r) {
    fprintf(OUT_FILE, "\tneg\t%s\n", reglist[r]);
    return r;
}

int cg_invert(int r) {
    fprintf(OUT_FILE, "\tnot\t%s\n", reglist[r]);
    return r;
}

int cg_lognot(int r) {
    fprintf(OUT_FILE, "\ttestq\t%s, %s\n", reglist[r], reglist[r]);
    fprintf(OUT_FILE, "\tsete\t%s\n", breglist[r]);
    fprintf(OUT_FILE, "\tmovzbq\t%s, %s\n", breglist[r], reglist[r]);

    return r;
}

int cg_tobool(int r, int op, int label) {
    fprintf(OUT_FILE, "\ttestq\t%s, %s\n", reglist[r], reglist[r]);
    if (op == A_IF || op == A_WHILE) {
        fprintf(OUT_FILE, "\tje\tL%d\n", label);
    } else {
        fprintf(OUT_FILE, "\tsetnz\t%s\n", breglist[r]);
        fprintf(OUT_FILE, "\tmovzbq\t%s, %s\n", breglist[r], reglist[r]);
    }

    return r;
}

int cg_and(int r1, int r2) {
    fprintf(OUT_FILE, "\tand\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_or(int r1, int r2) {
    fprintf(OUT_FILE, "\tor\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_xor(int r1, int r2) {
    fprintf(OUT_FILE, "\txor\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_sal(int r1, int r2) {
    fprintf(OUT_FILE, "\tmovb\t%s, %%cl\n", breglist[r2]);
    fprintf(OUT_FILE, "\tsalq\t%%cl, %s\n", reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_sar(int r1, int r2) {
    fprintf(OUT_FILE, "\tmovb\t%s, %%cl\n", breglist[r2]);
    fprintf(OUT_FILE, "\tsarq\t%%cl, %s\n", reglist[r1]);
    free_register(r2);
    return r1;
}

int cg_sal_n(int reg, int n) {
    fprintf(OUT_FILE, "\tsalq\t$%d, %s\n", n, reglist[reg]);

    return reg;
}

int cg_call(int id, int args_num) {
    int out_reg = alloc_register();

    fprintf(OUT_FILE, "\tcall\t%s@PLT\n", SYM_TAB[id].name);

    if (args_num > 6) {
        fprintf(OUT_FILE, "\taddq\t$%d, %%rsp\n", 8 * (args_num - 6));
    }

    fprintf(OUT_FILE, "\tmovq\t%%rax, %s\n", reglist[out_reg]);

    return out_reg;
}

void cg_copy_arg(int reg, int arg_pos) {
    if (arg_pos > 5) {
        fprintf(OUT_FILE, "\tpushq\t%s\n", reglist[reg]);
    } else {
        fprintf(OUT_FILE, "\tmovq\t%s, %s\n", reglist[reg], reglist[FREE_REG_NUM + arg_pos]);
    }
}

int cg_store_global_sym(int reg, int id) {
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
            fatald("Bad type in cg_store_global_sym()", SYM_TAB[id].ptype);
    }
    return reg;
}

int cg_store_local_sym(int reg, int id) {
    Symbol *sym = &SYM_TAB[id];
    switch (SYM_TAB[id].ptype) {
        case P_CHAR:
            fprintf(OUT_FILE, "\tmovb\t%s, %d(%%rbp)\n", breglist[reg], sym->posn);
            break;
        case P_INT:
            fprintf(OUT_FILE, "\tmovl\t%s, %d(%%rbp)\n", dreglist[reg], sym->posn);
            break;
        case P_LONG:
        case P_CHARPTR:
        case P_INTPTR:
        case P_LONGPTR:
            fprintf(OUT_FILE, "\tmovq\t%s, %d(%%rbp)\n", reglist[reg], SYM_TAB[id].posn);
            break;
        default:
            fatald("Bad type in cg_store_local_sym()", SYM_TAB[id].ptype);
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
    Symbol *sym = &SYM_TAB[id];

    if (sym->stype == S_FUNCTION) {
        return;
    }

    cg_data_section();

    fprintf(OUT_FILE,
            "\t.globl\t%s\n"
            "%s:",
            SYM_TAB[id].name,
            SYM_TAB[id].name);

    int size = cg_type_size(SYM_TAB[id].ptype);
    if (sym->stype == S_ARRAY) {
        size = cg_type_size((value_at(sym->ptype)));
    }

    for (int i = 0; i < sym->size; i++) {
        switch (size) {
            case 1:
                fprintf(OUT_FILE, "\t.byte\t0\n");
                break;
            case 2:
                fprintf(OUT_FILE, "\t.short\t0\n"); // or .word
                break;
            case 4:
                fprintf(OUT_FILE, "\t.int\t0\n"); // or .long
                break;
            case 8:
                fprintf(OUT_FILE, "\t.quad\t0\n");
                break;
            default:
                fatald("Bad type size in cg_new_sym()", size);
        }
    }
}

void cg_new_str(int l, char *str) {
    char *cptr;
    cg_label(l);

    for (cptr = str; *cptr; cptr++) {
        fprintf(OUT_FILE, "\t.byte\t%d\n", *cptr);
    }

    fprintf(OUT_FILE, "\t.byte\t0\n");
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

    Symbol *sym = &SYM_TAB[id];
    if (sym->class != C_GLOBAL) {
        fprintf(OUT_FILE,
                "\tleaq\t%d(%%rbp), %s\n",
                sym->posn,
                reglist[reg]);
    } else {
        fprintf(OUT_FILE,
                "\tleaq\t%s(%%rip), %s\n",
                sym->name,
                reglist[reg]);
    }

    return reg;
}

int cg_deref(int reg, int type) {
    switch (type) {
        case P_CHARPTR:
            fprintf(OUT_FILE, "\tmovzbq\t(%s), %s\n", reglist[reg], reglist[reg]);
            break;
        case P_INTPTR:
            fprintf(OUT_FILE, "\tmovslq\t(%s), %s\n", reglist[reg], reglist[reg]);
            break;
        case P_LONGPTR:
            fprintf(OUT_FILE, "\tmovq\t(%s), %s\n", reglist[reg], reglist[reg]);
            break;
        default:
            fatal("Bad type in cg_deref()");
    }

    return reg;
}

int cg_store_deref(int r1, int r2, int type) {
    switch (type) {
        case P_CHAR:
            fprintf(OUT_FILE, "\tmovzbq\t%s, %%rax\n", breglist[r1]);
            break;
        case P_INT:
            fprintf(OUT_FILE, "\tmovslq\t%s, %%rax\n", dreglist[r1]);
            break;
        case P_LONG:
            fprintf(OUT_FILE, "\tmovq\t%s, %%rax\n", reglist[r1]);
            break;
        default:
            fatald("Bad type in cg_store_deref()", type);
    }

    fprintf(OUT_FILE, "\tmovq\t%%rax, (%s)\n", reglist[r2]);

    return r1;
}
