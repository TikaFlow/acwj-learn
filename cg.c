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

static int local_offset, stack_offset, align = 8, spill_reg = 0;
static int freereg[FREE_REG_NUM];
static char *breglist[] = {"%r12b", "%r13b", "%r14b", "%r15b", "%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *wreglist[] = {"%r12w", "%r13w", "%r14w", "%r15w", "%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
static char *dreglist[] = {"%r12d", "%r13d", "%r14d", "%r15d", "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
static char *reglist[] = {"%r12", "%r13", "%r14", "%r15", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
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

static void cg_push_reg(char *reg) {
    fprintf(OUT_FILE, "\tpushq\t%s\n", reg);
}

static void cg_pop_reg(char *reg) {
    fprintf(OUT_FILE, "\tpopq\t%s\n", reg);
}

static void spill_all_regs() {
    for (int i = 0; i < FREE_REG_NUM; i++) {
        cg_push_reg(reglist[i]);
    }
}

static void restore_all_regs() {
    for (int i = FREE_REG_NUM - 1; i >= 0; i--) {
        cg_pop_reg(reglist[i]);
    }
}

static void cg_reset_local_offset() {
    local_offset = 0;
}

static int cg_get_local_offset(int size) {
    local_offset += size > align ? size : align;

    return -local_offset;
}

static void cg_set_stack_offset() {
    stack_offset = (local_offset + 0xF) & ~0xF;
}

int cg_align(int type, int offset, int direction) {
    int align_cut = align - 1;
    return (offset + align_cut * direction) & ~align_cut;
}

void cg_free_regs(int keep_reg) {
    for (int i = 0; i < FREE_REG_NUM; i++) {
        if (i != keep_reg) {
            freereg[i] = TRUE;
        }
    }
}

int cg_alloc_register() {
    int reg;
    for (reg = 0; reg < FREE_REG_NUM; reg++) {
        if (freereg[reg]) {
            freereg[reg] = FALSE;
            return reg;
        }
    }

    // when no free register, we spill one to stack
    reg = spill_reg++ % FREE_REG_NUM;
    cg_push_reg(reglist[reg]);

    return reg;
}

void cg_free_register(int reg) {
    if (freereg[reg]) {
        fprintf(stderr, "Error trying to free register %d\n", reg);
        exit(1);
    }

    if (spill_reg) {
        spill_reg--;
        cg_pop_reg(reglist[reg]);
        return;
    }
    freereg[reg] = TRUE;
}

void cg_pre_amble(char *file) {
    cg_free_regs(NO_REG);

    fprintf(OUT_FILE, "\t.file 1 \"%s\"\n", file);
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
            "\n"
            "# internal switch(expr) routine\n"
            "# %%rsi = switch table, %%rax = expr\n"
            "# from SubC: http://www.t3x.org/subc/\n");
    cg_text_section();
    fprintf(OUT_FILE,
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
    set_func_ptr(sym);
    char *name = sym->name;
    Symbol *local;
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

    for (local = sym->first; local; local = local->next) {
        // copy all params to register
        if (local->class == C_PARAM) {
            if (cnt <= 6) {
                local->posn = cg_get_local_offset(local->size);
                cg_store_local_sym(param_reg++, local);
            } else {
                local->posn = param_offset;
                param_offset += local->size > align ? local->size : align;
            }
            cnt++;
        } else {
            local->posn = cg_get_local_offset(local->size);
        }
    }

    cg_set_stack_offset();
    fprintf(OUT_FILE, "\tsubq\t$%d, %%rsp\n", stack_offset);
    spill_all_regs(); // r12-r15 are callee saved
}

void cg_func_post_amble(Symbol *sym) {
    cg_label(sym->end_label);
    restore_all_regs(); // r12-r15 are callee saved
    fprintf(OUT_FILE,
            "\taddq\t$%d, %%rsp\n"
            "\tpopq\t%%rbp\n"
            "\tret\n",
            stack_offset);
    reset_func_ptr();
}

int cg_load_int(long value) {
    int reg = cg_alloc_register();

    fprintf(OUT_FILE, "\tmovq\t$%ld, %s\n", value, reglist[reg]);
    return reg;
}

static char *get_address(Symbol *sym, int is_global) {
    char *addr = malloc(sizeof(char) * 0x20);
    if (is_global) {
        snprintf(addr, 0x20, "%s", sym->name);
    } else {
        snprintf(addr, 0x20, "%d(%%rbp)", sym->posn);
    }

    return addr;
}

static int cg_load_sym(Symbol *sym, int op, int is_global) {
    char *addr = get_address(sym, is_global);
    char *base = is_global ? "(%rip)" : "";
    char *add_cmd = NULL;
    char *mov_cmd = NULL;
    int reg = cg_alloc_register();
    int offset = 1;
    int type_size = cg_type_size(sym->ptype);

    if (type_size == 8 && is_ptr(sym->ptype)) {
        offset = size_of_type(value_at(sym->ptype), sym->ctype);
    }
    if (op == A_PREDEC || op == A_POSTDEC) {
        offset = -offset;
    }

    switch (type_size) {
        case 1:
            add_cmd = "addb";
            mov_cmd = "movsbq";
            break;
        case 2:
            add_cmd = "addw";
            mov_cmd = "movswq";
            break;
        case 4:
            add_cmd = "addl";
            mov_cmd = "movslq";
            break;
        case 8:
            add_cmd = "addq";
            mov_cmd = "movq";
            break;
        default:
            fatals("Bad type when load symbol", get_name(V_PTYPE, sym->ptype));
    }

    if (op == A_PREINC || op == A_PREDEC) {
        fprintf(OUT_FILE, "\t%s\t$%d, %s\n", add_cmd, offset, addr);
    }
    fprintf(OUT_FILE, "\t%s\t%s%s, %s\n", mov_cmd, addr, base, reglist[reg]);
    if (op == A_POSTINC || op == A_POSTDEC) {
        fprintf(OUT_FILE, "\t%s\t$%d, %s\n", add_cmd, offset, addr);
    }
    return reg;
}

int cg_load_global_sym(Symbol *sym, int op) {
    return cg_load_sym(sym, op, TRUE);
}

int cg_load_local_sym(Symbol *sym, int op) {
    return cg_load_sym(sym, op, FALSE);
}

int cg_load_str(int label) {
    int reg = cg_alloc_register();
    fprintf(OUT_FILE, "\tleaq\tL%d(%%rip), %s\n", label, reglist[reg]);
    return reg;
}

int cg_add(int r1, int r2) {
    fprintf(OUT_FILE, "\taddq\t%s, %s\n", reglist[r2], reglist[r1]);
    cg_free_register(r2);
    return r1;
}

int cg_sub(int r1, int r2) {
    fprintf(OUT_FILE, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
    cg_free_register(r2);
    return r1;
}

int cg_mul(int r1, int r2) {
    fprintf(OUT_FILE, "\timulq\t%s, %s\n", reglist[r2], reglist[r1]);
    cg_free_register(r2);
    return r1;
}

static int cg_div_mod(int r1, int r2, int is_div) {
    fprintf(OUT_FILE, "\tmovq\t%s,%%rax\n", reglist[r1]);
    fprintf(OUT_FILE, "\tcqto\n");
    fprintf(OUT_FILE, "\tidivq\t%s\n", reglist[r2]);
    if (is_div) {
        fprintf(OUT_FILE, "\tmovq\t%%rax, %s\n", reglist[r1]);
    } else {
        fprintf(OUT_FILE, "\tmovq\t%%rdx, %s\n", reglist[r1]);
    }

    cg_free_register(r2);
    return r1;
}

int cg_div(int r1, int r2) {
    return cg_div_mod(r1, r2, TRUE);
}

int cg_mod(int r1, int r2) {
    return cg_div_mod(r1, r2, FALSE);
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
    switch (op) {
        case A_IF:
        case A_WHILE:
        case A_LOGAND:
            fprintf(OUT_FILE, "\tje\tL%d\n", label);
            break;
        case A_LOGOR:
            fprintf(OUT_FILE, "\tjne\tL%d\n", label);
            break;
        default:
            fprintf(OUT_FILE, "\tsetnz\t%s\n", breglist[r]);
            fprintf(OUT_FILE, "\tmovzbq\t%s, %s\n", breglist[r], reglist[r]);
    }

    return r;
}

int cg_load_bool(int reg, int val) {
    fprintf(OUT_FILE, "\tmovq\t$%d, %s\n", val, reglist[reg]);
    return reg;
}

int cg_and(int r1, int r2) {
    fprintf(OUT_FILE, "\tand\t%s, %s\n", reglist[r2], reglist[r1]);
    cg_free_register(r2);
    return r1;
}

int cg_or(int r1, int r2) {
    fprintf(OUT_FILE, "\tor\t%s, %s\n", reglist[r2], reglist[r1]);
    cg_free_register(r2);
    return r1;
}

int cg_xor(int r1, int r2) {
    fprintf(OUT_FILE, "\txor\t%s, %s\n", reglist[r2], reglist[r1]);
    cg_free_register(r2);
    return r1;
}

int cg_sal(int r1, int r2) {
    fprintf(OUT_FILE, "\tmovb\t%s, %%cl\n", breglist[r2]);
    fprintf(OUT_FILE, "\tsalq\t%%cl, %s\n", reglist[r1]);
    cg_free_register(r2);
    return r1;
}

int cg_sar(int r1, int r2) {
    fprintf(OUT_FILE, "\tmovb\t%s, %%cl\n", breglist[r2]);
    fprintf(OUT_FILE, "\tsarq\t%%cl, %s\n", reglist[r1]);
    cg_free_register(r2);
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
                fprintf(OUT_FILE, "\tmovw\t%s, %s(%%rip)\n", wreglist[reg], sym->name);
                break;
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
                fprintf(OUT_FILE, "\tmovw\t%s, %d(%%rbp)\n", wreglist[reg], sym->posn);
                break;
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
    int size, type, i;
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
        size = size_of_type(sym->ptype, sym->ctype);
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
                fprintf(OUT_FILE, "\t.zero\t%d\n", size);
        }
    }
}

void cg_new_str(int label, char *str) {
    char *cptr;
    if (label != NO_LABEL) {
        cg_label(label);
    }

    for (cptr = str; *cptr; cptr++) {
        fprintf(OUT_FILE, "\t.byte\t%d\n", *cptr);
    }
}

void cg_new_str_end() {
    fprintf(OUT_FILE, "\t.byte\t0\n");
}

void cg_label(int l) {
    fprintf(OUT_FILE, "L%d:\n", l);
}

int cg_jump(int l) {
    fprintf(OUT_FILE, "\tjmp\tL%d\n", l);

    return NO_REG;
}

static void cg_compare(ASTnode *node, int r1, int r2) {
    int op = node->op;
    int type = node->left->type;
    int size = cg_type_size(type);

    if (op < A_EQ || op > A_GE) {
        fatal("Bad ASTop in cg_compare_and_set()");
    }

    switch (size) {
        case 1:
            fprintf(OUT_FILE, "\tcmpb\t%s, %s\n", breglist[r2], breglist[r1]);
            break;
        case 2:
            fprintf(OUT_FILE, "\tcmpw\t%s, %s\n", wreglist[r2], wreglist[r1]);
            break;
        case 4:
            fprintf(OUT_FILE, "\tcmpl\t%s, %s\n", dreglist[r2], dreglist[r1]);
            break;
        case 8:
            fprintf(OUT_FILE, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
            break;
        default:
            break; // make compiler happy
    }
}

int cg_compare_and_set(ASTnode *node, int r1, int r2) {
    cg_compare(node, r1, r2);

    fprintf(OUT_FILE, "\t%s\t%s\n", cmplist[node->op - A_EQ], breglist[r2]);
    fprintf(OUT_FILE, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);
    cg_free_register(r1);

    return r2;
}

int cg_compare_and_jump(ASTnode *node, int r1, int r2, int l) {
    cg_compare(node, r1, r2);

    fprintf(OUT_FILE, "\t%s\tL%d\n", invcmplist[node->op - A_EQ], l);
    cg_free_register(r1);
    cg_free_register(r2);

    return NO_REG;
}

int cg_widen(int r, int old_type, int new_type) {
    return r;
}

int cg_return(int reg, Symbol *sym) {
    if (reg != NO_REG) {
        if (is_ptr(sym->ptype)) {
            fprintf(OUT_FILE, "\tmovq\t%s, %%rax\n", reglist[reg]);
        } else {
            switch (sym->ptype) {
                case P_VOID:
                    break;
                case P_CHAR:
                    fprintf(OUT_FILE, "\tmovsbq\t%s, %%rax\n", breglist[reg]);
                    break;
                case P_SHORT:
                    fprintf(OUT_FILE, "\tmovswq\t%s, %%rax\n", wreglist[reg]);
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
        cg_free_register(reg);
    }
    cg_jump(sym->end_label);

    return NO_REG;
}

int cg_address(Symbol *sym) {
    int reg = cg_alloc_register();

    if (sym->class == C_GLOBAL || sym->class == C_STATIC || sym->class == C_EXTERN) {
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
            fprintf(OUT_FILE, "\tmovsbq\t(%s), %s\n", reglist[reg], reglist[reg]);
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
            fprintf(OUT_FILE, "\tmovb\t%s, (%s)\n", breglist[r1], reglist[r2]);
            break;
        case 2:
            fprintf(OUT_FILE, "\tmovw\t%s, (%s)\n", wreglist[r1], reglist[r2]);
            break;
        case 4:
            fprintf(OUT_FILE, "\tmovl\t%s, (%s)\n", dreglist[r1], reglist[r2]);
            break;
        case 8:
            fprintf(OUT_FILE, "\tmovq\t%s, (%s)\n", reglist[r1], reglist[r2]);
            break;
        default:
            fatals("Bad type in cg_store_deref()", get_name(V_PTYPE, type));
    }

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

void cg_line_num(int line_num) {
    fprintf(OUT_FILE, "\t.loc 1 %d 0\n", line_num);
}
