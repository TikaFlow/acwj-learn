//
// Created by tika on 24-5-21.
//
// generate AT&T assembly code

#include "data.h"
#include "decl.h"

static int freereg[4];
static char *reglist[] = {"%r8", "%r9", "%r10", "%r11"};
static char *breglist[] = {"%r8b", "%r9b", "%r10b", "%r11b"};
static char *cmplist[] = {"sete", "setne", "setl", "setg", "setle", "setge"};
static char *invcmplist[] = {"jne", "je", "jge", "jle", "jg", "jl"};

void cgfreeregs() {
    for (int i = 0; i < 4; i++) {
        freereg[i] = 1;
    }
}

static int alloc_register() {
    for (int i = 0; i < 4; i++) {
        if (freereg[i]) {
            freereg[i] = 0;
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
    freereg[reg] = 1;
}

void cgpreamble() {
    cgfreeregs();
    fputs("\t.text\n"
          ".LC0:\n"
          "\t.string\t\"%d\\n\"\n"
          "printint:\n"
          "\tpushq\t%rbp\n"
          "\tmovq\t%rsp, %rbp\n"
          "\tmovl\t%edi, %esi\n"
          "\tleaq\t.LC0(%rip), %rdi\n"
          "\tmovl\t$0, %eax\n"
          "\tcall\tprintf@PLT\n"
          "\tnop\n"
          "\tleave\n"
          "\tret\n"
          "\n",
          OUT_FILE);
}

void cgfuncpostamble() {
    fputs("\tmovl\t$0, %eax\n"
          "\tpopq\t%rbp\n"
          "\tret\n",
          OUT_FILE);
}

void cgfuncpreamble(char *name) {
    fprintf(OUT_FILE,
            "\t.text\n"
            "\t.globl\t%s\n"
            "\t.type\t%s, @function\n"
            "%s:\n"
            "\tpushq\t%%rbp\n"
            "\tmovq\t%%rsp, %%rbp\n",
            name, name, name);
}

int cgloadint(int value) {
    int reg = alloc_register();

    fprintf(OUT_FILE, "\tmovq\t$%d, %s\n", value, reglist[reg]);
    return reg;
}

int cgloadglob(char *identifier) {
    int reg = alloc_register();
    fprintf(OUT_FILE, "\tmovq\t%s(%%rip), %s\n", identifier, reglist[reg]);
    return reg;
}

int cgadd(int r1, int r2) {
    fprintf(OUT_FILE, "\taddq\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cgsub(int r1, int r2) {
    fprintf(OUT_FILE, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cgmul(int r1, int r2) {
    fprintf(OUT_FILE, "\timulq\t%s, %s\n", reglist[r2], reglist[r1]);
    free_register(r2);
    return r1;
}

int cgdiv(int r1, int r2) {
    fprintf(OUT_FILE, "\tmovq\t%s,%%rax\n", reglist[r1]);
    fprintf(OUT_FILE, "\tcqto\n");
    fprintf(OUT_FILE, "\tidivq\t%s\n", reglist[r2]);
    fprintf(OUT_FILE, "\tmovq\t%%rax, %s\n", reglist[r1]);
    free_register(r2);
    return r1;
}

void cgprintint(int reg) {
    fprintf(OUT_FILE, "\tmovq\t%s,%%rdi\n", reglist[reg]);
    fprintf(OUT_FILE, "\tcall\tprintint\n");
    free_register(reg);
}

int cgstorglob(int reg, char *identifier) {
    fprintf(OUT_FILE, "\tmovq\t%s, %s(%%rip)\n", reglist[reg], identifier);
    return reg;
}

void cgglobsym(char *sym) {
    fprintf(OUT_FILE, "\t.comm\t%s, 8, 8\n", sym);
}

void cglabel(int l) {
    fprintf(OUT_FILE, "L%d:\n", l);
}

void cgjump(int l) {
    fprintf(OUT_FILE, "\tjmp\tL%d\n", l);
}

int cgcompare_and_set(int ASTop, int r1, int r2) {
    if (ASTop < A_EQ || ASTop > A_GE) {
        fatal("Bad ASTop in cgcompare_and_set()");
    }

    fprintf(OUT_FILE, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
    fprintf(OUT_FILE, "\t%s\t%s\n", cmplist[ASTop - A_EQ], reglist[r2]);
    fprintf(OUT_FILE, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);
    free_register(r1);

    return r2;
}

int cgcompare_and_jump(int ASTop, int r1, int r2, int l) {
    if (ASTop < A_EQ || ASTop > A_GE) {
        fatal("Bad ASTop in cgcompare_and_jump()");
    }

    fprintf(OUT_FILE, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
    fprintf(OUT_FILE, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], l);
    cgfreeregs();

    return NO_REG;
}
