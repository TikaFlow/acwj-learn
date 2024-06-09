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

static int local_offset, stack_offset, ALIGN = 0b11;
static int freereg[FREE_REG_NUM];
static char *breglist[] = {"%r10b", "%r11b", "%r12b", "%r13b", "%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *dreglist[] = {"%r10d", "%r11d", "%r12d", "%r13d", "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
static char *reglist[] = {"%r10", "%r11", "%r12", "%r13", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char *cmplist[] = {"sete", "setne", "setl", "setg", "setle", "setge"};
static char *invcmplist[] = {"jne", "je", "jge", "jle", "jg", "jl"};

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

static void cg_set_stack_offset() {
    stack_offset = (local_offset + 0xF) & ~0xF;
}

int cg_align(int type, int offset, int direction) {
    return type == P_CHAR ? offset : (offset + ALIGN * direction) & ~ALIGN;
}

void cg_free_regs(int keep_reg) {
    for (int i = 0; i < FREE_REG_NUM; i++) {
        if (i != keep_reg) {
            freereg[i] = TRUE;
        }
    }
}

int cg_alloc_register() {
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
    cg_free_regs(NO_REG);
/*
# internal switch(expr) routine
# %rsi = switch table, %rax = expr

switch:
        pushq   %rsi            # Save %rsi
        movq    %rdx,%rsi       # Base of jump table -> %rsi
        movq    %rax,%rbx       # Switch value -> %rbx
        cld                     # Clear direction flag
        lodsq                   # Load count of cases into %rcx,
        movq    %rax,%rcx       # incrementing %rsi in the process
next:
        lodsq                   # Get the case value into %rdx
        movq    %rax,%rdx
        lodsq                   # and the label address into %rax
        cmpq    %rdx,%rbx       # Does switch value matches the case?
        jnz     no              # No, jump over this code
        popq    %rsi            # Restore %rsi
        jmp     *%rax           # and jump to the chosen case
no:
        loop    next            # Loop for the number of cases
        lodsq                   # Out of loop, load default label address
        popq    %rsi            # Restore %rsi
        jmp     *%rax           # and jump to the default case
 */
    fprintf(OUT_FILE,
            "# internal switch(expr) routine\n"
            "# %%rsi = switch table, %%rax = expr\n"
            "# from SubC: http://www.t3x.org/subc/\n"
            "\n"
            "switch:\n"
            "\tpushq\t%%rsi\n"
            "\tmovq\t%%rdx, %%rsi\n"
            "\tmovq\t%%rax, %%rbx\n"
            "\tcld\n"
            "\tlodsq\n"
            "\tmovq\t%%rax, %%rcx\n"
            "next:\n"
            "\tlodsq\n"
            "\tmovq\t%%rax, %%rdx\n"
            "\tlodsq\n"
            "\tcmpq\t%%rdx, %%rbx\n"
            "\tjnz\tno\n"
            "\tpopq\t%%rsi\n"
            "\tjmp\t*%%rax\n"
            "no:\n"
            "\tloop\tnext\n"
            "\tlodsq\n"
            "\tpopq\t%%rsi\n"
            "\tjmp\t*%%rax\n\n");
}

// Nothing to do
void cg_post_amble() {
}

void cg_func_pre_amble(Symbol *sym) {
    char *name = sym->name;
    Symbol *param, *local;
    int cnt = 1, param_offset = 0x10, param_reg = FREE_REG_NUM;
    cg_text_section();
    cg_reset_local_offset();

    if (sym->class == C_GLOBAL) {
        fprintf(OUT_FILE, "\t.globl\t%s\n", name);
    }

    fprintf(OUT_FILE,
            "\t.type\t%s, @function\n"
            "%s:\n"
            "\tpushq\t%%rbp\n"
            "\tmovq\t%%rsp, %%rbp\n",
            name, name);

    // copy all params to register
    for (param = sym->first; param; cnt++, param = param->next) {
        if (cnt <= 6) {
            param->posn = cg_get_local_offset(param->ptype);
            cg_store_local_sym(param_reg++, param);
        } else {
            param->posn = param_offset;
            param_offset += 8;
        }
    }

    for (local = LOCAL_HEAD; local; local = local->next) {
        local->posn = cg_get_local_offset(local->ptype);
    }

    cg_set_stack_offset();
    fprintf(OUT_FILE, "\tsubq\t$%d, %%rsp\n", stack_offset);
}

void cg_func_post_amble(Symbol *sym) {
    cg_label(sym->end_label);
    fprintf(OUT_FILE,
            "\taddq\t$%d, %%rsp\n"
            "\tpopq\t%%rbp\n"
            "\tret\n",
            stack_offset);
}

int cg_load_int(long value) {
    int reg = cg_alloc_register();

    fprintf(OUT_FILE, "\tmovq\t$%ld, %s\n", value, reglist[reg]);
    return reg;
}

int cg_load_global_sym(Symbol *sym, int op) {
    int reg = cg_alloc_register();
    if (cg_type_size(sym->ptype) == 8) {
        if (op == A_PREINC) {
            fprintf(OUT_FILE, "\tincq\t%s\n", sym->name);
        }
        if (op == A_PREDEC) {
            fprintf(OUT_FILE, "\tdecq\t%s\n", sym->name);
        }
        fprintf(OUT_FILE, "\tmovq\t%s(%%rip), %s\n", sym->name, reglist[reg]);
        if (op == A_POSTINC) {
            fprintf(OUT_FILE, "\tincq\t%s\n", sym->name);
        }
        if (op == A_POSTDEC) {
            fprintf(OUT_FILE, "\tdecq\t%s\n", sym->name);
        }
    } else {
        switch (sym->ptype) {
            case P_CHAR:
                if (op == A_PREINC) {
                    fprintf(OUT_FILE, "\tincb\t%s\n", sym->name);
                }
                if (op == A_PREDEC) {
                    fprintf(OUT_FILE, "\tdecb\t%s\n", sym->name);
                }
                fprintf(OUT_FILE, "\tmovzbq\t%s(%%rip), %s\n", sym->name, reglist[reg]);
                if (op == A_POSTINC) {
                    fprintf(OUT_FILE, "\tincb\t%s\n", sym->name);
                }
                if (op == A_POSTDEC) {
                    fprintf(OUT_FILE, "\tdecb\t%s\n", sym->name);
                }
                break;
            case P_SHORT:
                if (op == A_PREINC) {
                    fprintf(OUT_FILE, "\tincw\t%s\n", sym->name);
                }
                if (op == A_PREDEC) {
                    fprintf(OUT_FILE, "\tdecw\t%s\n", sym->name);
                }
                fprintf(OUT_FILE, "\tmovswq\t%s(%%rip), %s\n", sym->name, reglist[reg]);
                if (op == A_POSTINC) {
                    fprintf(OUT_FILE, "\tincw\t%s\n", sym->name);
                }
                if (op == A_POSTDEC) {
                    fprintf(OUT_FILE, "\tdecw\t%s\n", sym->name);
                }
                break;
            case P_INT:
                if (op == A_PREINC) {
                    fprintf(OUT_FILE, "\tincl\t%s\n", sym->name);
                }
                if (op == A_PREDEC) {
                    fprintf(OUT_FILE, "\tdecl\t%s\n", sym->name);
                }
                fprintf(OUT_FILE, "\tmovslq\t%s(%%rip), %s\n", sym->name, reglist[reg]);
                if (op == A_POSTINC) {
                    fprintf(OUT_FILE, "\tincl\t%s\n", sym->name);
                }
                if (op == A_POSTDEC) {
                    fprintf(OUT_FILE, "\tdecl\t%s\n", sym->name);
                }
                break;
            default:
                fatals("Bad type in cg_load_global_sym()", get_name(V_PTYPE, sym->ptype));
        }
    }
    return reg;
}

int cg_load_local_sym(Symbol *sym, int op) {
    int reg = cg_alloc_register();

    if (cg_type_size(sym->ptype) == 8) {

        if (op == A_PREINC) {
            fprintf(OUT_FILE, "\tincq\t%d(%%rbp)\n", sym->posn);
        }
        if (op == A_PREDEC) {
            fprintf(OUT_FILE, "\tdecq\t%d(%%rbp)\n", sym->posn);
        }
        fprintf(OUT_FILE, "\tmovq\t%d(%%rbp), %s\n", sym->posn, reglist[reg]);
        if (op == A_POSTINC) {
            fprintf(OUT_FILE, "\tincq\t%d(%%rbp)\n", sym->posn);
        }
        if (op == A_POSTDEC) {
            fprintf(OUT_FILE, "\tdecq\t%d(%%rbp)\n", sym->posn);
        }
    } else {
        switch (sym->ptype) {
            case P_CHAR:
                if (op == A_PREINC) {
                    fprintf(OUT_FILE, "\tincb\t%d(%%rbp)\n", sym->posn);
                }
                if (op == A_PREDEC) {
                    fprintf(OUT_FILE, "\tdecb\t%d(%%rbp)\n", sym->posn);
                }
                fprintf(OUT_FILE, "\tmovzbq\t%d(%%rbp), %s\n", sym->posn, reglist[reg]);
                if (op == A_POSTINC) {
                    fprintf(OUT_FILE, "\tincb\t%d(%%rbp)\n", sym->posn);
                }
                if (op == A_POSTDEC) {
                    fprintf(OUT_FILE, "\tdecb\t%d(%%rbp)\n", sym->posn);
                }
                break;
            case P_SHORT:
                if (op == A_PREINC) {
                    fprintf(OUT_FILE, "\tincw\t%d(%%rbp)\n", sym->posn);
                }
                if (op == A_PREDEC) {
                    fprintf(OUT_FILE, "\tdecw\t%d(%%rbp)\n", sym->posn);
                }
                fprintf(OUT_FILE, "\tmovswq\t%d(%%rbp), %s\n", sym->posn, reglist[reg]);
                if (op == A_POSTINC) {
                    fprintf(OUT_FILE, "\tincw\t%d(%%rbp)\n", sym->posn);
                }
                if (op == A_POSTDEC) {
                    fprintf(OUT_FILE, "\tdecw\t%d(%%rbp)\n", sym->posn);
                }
                break;
            case P_INT:
                if (op == A_PREINC) {
                    fprintf(OUT_FILE, "\tincl\t%d(%%rbp)\n", sym->posn);
                }
                if (op == A_PREDEC) {
                    fprintf(OUT_FILE, "\tdecl\t%d(%%rbp)\n", sym->posn);
                }
                fprintf(OUT_FILE, "\tmovslq\t%d(%%rbp), %s\n", sym->posn, reglist[reg]);
                if (op == A_POSTINC) {
                    fprintf(OUT_FILE, "\tincl\t%d(%%rbp)\n", sym->posn);
                }
                if (op == A_POSTDEC) {
                    fprintf(OUT_FILE, "\tdecl\t%d(%%rbp)\n", sym->posn);
                }
                break;
            default:
                fatals("Bad type in cg_load_local_sym()", get_name(V_PTYPE, sym->ptype));
        }
    }
    return reg;
}

int cg_load_str(int label) {
    int reg = cg_alloc_register();
    fprintf(OUT_FILE, "\tleaq\tL%d(%%rip), %s\n", label, reglist[reg]);
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

int cg_logor(int r1, int r2) {
    int ltrue = gen_label(), lend = gen_label();

    // test r1 and jump to ltrue if true
    fprintf(OUT_FILE, "\ttestq\t%s, %s\n", reglist[r1], reglist[r1]);
    fprintf(OUT_FILE, "\tjne\tL%d\n", ltrue);

    // test r2 and jump to ltrue if true
    fprintf(OUT_FILE, "\ttestq\t%s, %s\n", reglist[r2], reglist[r2]);
    fprintf(OUT_FILE, "\tjne\tL%d\n", ltrue);

    // didn't jump, so result is false
    fprintf(OUT_FILE, "\tmovq\t$0, %s\n", reglist[r1]);
    fprintf(OUT_FILE, "\tjmp\tL%d\n", lend);

    // someone jump to here, so r1 is true
    cg_label(ltrue);
    fprintf(OUT_FILE, "\tmovq\t$1, %s\n", reglist[r1]);
    cg_label(lend);

    return r1;
}

int cg_logand(int r1, int r2){
    int lfalse = gen_label(), lend = gen_label();

    // test r1 and jump to lfalse if false
    fprintf(OUT_FILE, "\ttestq\t%s, %s\n", reglist[r1], reglist[r1]);
    fprintf(OUT_FILE, "\tje\tL%d\n", lfalse);

    // test r2 and jump to lfalse if false
    fprintf(OUT_FILE, "\ttestq\t%s, %s\n", reglist[r2], reglist[r2]);
    fprintf(OUT_FILE, "\tje\tL%d\n", lfalse);

    // didn't jump, so result is true
    fprintf(OUT_FILE, "\tmovq\t$1, %s\n", reglist[r1]);
    fprintf(OUT_FILE, "\tjmp\tL%d\n", lend);

    // someone jump to here, so r1 is false
    cg_label(lfalse);
    fprintf(OUT_FILE, "\tmovq\t$0, %s\n", reglist[r1]);
    cg_label(lend);

    return r1;
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

int cg_call(Symbol *sym, int args_num) {
    int out_reg = cg_alloc_register();

    fprintf(OUT_FILE, "\tcall\t%s@PLT\n", sym->name);

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

int cg_store_global_sym(int reg, Symbol *sym) {
    if (cg_type_size(sym->ptype) == 8) {
        fprintf(OUT_FILE, "\tmovq\t%s, %s(%%rip)\n", reglist[reg], sym->name);
    } else {
        switch (sym->ptype) {
            case P_CHAR:
                fprintf(OUT_FILE, "\tmovb\t%s, %s(%%rip)\n", breglist[reg], sym->name);
                break;
            case P_SHORT:
            case P_INT:
                fprintf(OUT_FILE, "\tmovl\t%s, %s(%%rip)\n", dreglist[reg], sym->name);
                break;
            default:
                fatals("Bad type in cg_store_global_sym()", get_name(V_PTYPE, sym->ptype));
        }
    }
    return reg;
}

int cg_store_local_sym(int reg, Symbol *sym) {
    if (cg_type_size(sym->ptype) == 8) {
        fprintf(OUT_FILE, "\tmovq\t%s, %d(%%rbp)\n", reglist[reg], sym->posn);
    } else {
        switch (sym->ptype) {
            case P_CHAR:
                fprintf(OUT_FILE, "\tmovb\t%s, %d(%%rbp)\n", breglist[reg], sym->posn);
                break;
            case P_SHORT:
            case P_INT:
                fprintf(OUT_FILE, "\tmovl\t%s, %d(%%rbp)\n", dreglist[reg], sym->posn);
                break;
            default:
                fatals("Bad type in cg_store_local_sym()", get_name(V_PTYPE, sym->ptype));
        }
    }
    return reg;
}

int cg_type_size(int type) {
    if (is_ptr(type)) {
        return 8;
    }
    switch (type) {
        case P_CHAR:
            return 1;
        case P_SHORT:
            return 2;
        case P_INT:
            return 4;
        case P_LONG:
            return 8;
        default:
            fatals("Bad type in cg_type_size()", get_name(V_PTYPE, type));
    }

    return 0; // keep compiler happy
}

void cg_new_sym(Symbol *sym) {
    int size, type, i, j;
    long init_value;

    if (!sym || sym->stype == S_FUNCTION) {
        return;
    }

    cg_data_section();
    if (sym->class == C_GLOBAL) {
        fprintf(OUT_FILE, "\t.globl\t%s\n", sym->name);
    }
    fprintf(OUT_FILE, "%s:\n", sym->name);

    if (sym->stype == S_ARRAY) {
        size = size_of_type(value_at(sym->ptype), sym->ctype); // howto? if a struct array
        type = value_at(sym->ptype);
    } else {
        size = sym->size;
        type = sym->ptype;
    }

    for (i = 0; i < sym->n_elem; i++) {
        init_value = 0;
        if (sym->init_list) {
            init_value = sym->init_list[i];
        }
        switch (size) {
            case 1:
                fprintf(OUT_FILE, "\t.byte\t%d\n", (char) init_value);
                break;
            case 2:
                fprintf(OUT_FILE, "\t.short\t%d\n", (short) init_value); // or .word
                break;
            case 4:
                fprintf(OUT_FILE, "\t.int\t%d\n", (int) init_value); // or .long
                break;
            case 8:
                if (sym->init_list && type == pointer_to(P_CHAR) && init_value != 0) {
                    fprintf(OUT_FILE, "\t.quad\tL%d\n", (int) init_value);
                    break;
                }
                fprintf(OUT_FILE, "\t.quad\t%ld\n", init_value);
                break;
            default:
                for (j = 0; j < size; j++) {
                    fprintf(OUT_FILE, "\t.byte\t0\n");
                }
        }
    }
}

void cg_new_str(int label, char *str) {
    char *cptr;
    cg_label(label);

    for (cptr = str; *cptr; cptr++) {
        fprintf(OUT_FILE, "\t.byte\t%d\n", *cptr);
    }

    fprintf(OUT_FILE, "\t.byte\t0\n");
}

void cg_label(int l) {
    fprintf(OUT_FILE, "L%d:\n", l);
}

int cg_jump(int l) {
    fprintf(OUT_FILE, "\tjmp\tL%d\n", l);

    return NO_REG;
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
    cg_free_regs(NO_REG);

    return NO_REG;
}

int cg_widen(int r, int old_type, int new_type) {
    return r;
}

int cg_return(int reg, Symbol *sym) {
    if (is_ptr(sym->ptype)) {
        fprintf(OUT_FILE, "\tmovq\t%s, %%rax\n", reglist[reg]);
    } else {
        switch (sym->ptype) {
            case P_CHAR:
                fprintf(OUT_FILE, "\tmovzbq\t%s, %%rax\n", breglist[reg]);
                break;
            case P_SHORT:
                fprintf(OUT_FILE, "\tmovswq\t%s, %%rax\n", dreglist[reg]);
                break;
            case P_INT:
                fprintf(OUT_FILE, "\tmovslq\t%s, %%rax\n", dreglist[reg]);
                break;
            case P_LONG:
                fprintf(OUT_FILE, "\tmovq\t%s, %%rax\n", reglist[reg]);
                break;
            default:
                fatals("Bad function type in cg_return()", get_name(V_PTYPE, sym->ptype));
        }
    }
    cg_jump(sym->end_label);

    return NO_REG;
}

int cg_address(Symbol *sym) {
    int reg = cg_alloc_register();

    if (sym->class == C_GLOBAL || sym->class == C_STATIC) {
        fprintf(OUT_FILE, "\tleaq\t%s(%%rip), %s\n", sym->name, reglist[reg]);
    } else {
        fprintf(OUT_FILE, "\tleaq\t%d(%%rbp), %s\n", sym->posn, reglist[reg]);
    }

    return reg;
}

int cg_deref(int reg, int type) {
    int size = cg_type_size(value_at(type));

    switch (size) {
        case 1:
            fprintf(OUT_FILE, "\tmovzbq\t(%s), %s\n", reglist[reg], reglist[reg]);
            break;
        case 2:
            fprintf(OUT_FILE, "\tmovswq\t(%s), %s\n", reglist[reg], reglist[reg]);
        case 4:
            fprintf(OUT_FILE, "\tmovslq\t(%s), %s\n", reglist[reg], reglist[reg]);
            break;
        case 8:
            fprintf(OUT_FILE, "\tmovq\t(%s), %s\n", reglist[reg], reglist[reg]);
            break;
        default:
            fatal("Bad type in cg_deref()");
    }

    return reg;
}

int cg_store_deref(int r1, int r2, int type) {
    int size = cg_type_size(type);

    switch (size) {
        case 1:
            fprintf(OUT_FILE, "\tmovzbq\t%s, %%rax\n", breglist[r1]);
            break;
        case 2:
            fprintf(OUT_FILE, "\tmovswq\t%s, %%rax\n", dreglist[r1]);
            break;
        case 4:
            fprintf(OUT_FILE, "\tmovslq\t%s, %%rax\n", dreglist[r1]);
            break;
        case 8:
            fprintf(OUT_FILE, "\tmovq\t%s, %%rax\n", reglist[r1]);
            break;
        default:
            fatals("Bad type in cg_store_deref()", get_name(V_PTYPE, type));
    }

    fprintf(OUT_FILE, "\tmovq\t%%rax, (%s)\n", reglist[r2]);

    return r1;
}

void cg_switch(int reg, int case_cnt, int *case_label, int *case_val, int dft_label) {
    int i, tab_label = gen_label();

    fprintf(OUT_FILE,
            "\tmovq\t%s, %%rax\n"
            "\tleaq\tL%d(%%rip), %%rdx\n"
            "\tjmp\tswitch\n",
            reglist[reg], tab_label);

    cg_label(tab_label);
    fprintf(OUT_FILE, "\t.quad\t%d\n", case_cnt);
    for (i = 0; i < case_cnt; i++) {
        fprintf(OUT_FILE, "\t.quad\t%d, L%d\n", case_val[i], case_label[i]);
    }
    fprintf(OUT_FILE, "\t.quad\tL%d\n", dft_label);
}

void cg_mov_reg(int r1, int r2) {
    fprintf(OUT_FILE, "\tmovq\t%s, %s\n", reglist[r1], reglist[r2]);
}
