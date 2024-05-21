//
// Created by tika on 24-5-21.
//
// generate AT&T assembly code

#include "data.h"
#include "decl.h"

static int freereg[4];
static char *reglist[] = {"%r8", "%r9", "%r10", "%r11"};

void freeall_registers() {
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
    freeall_registers();
    fputs("\t.text\n"
          ".LC0:\n"
          "\t.string\t\"%d\\n\"\n"
          "printint:\n"
          "\tpushq\t%rbp\n"
          "\tmovq\t%rsp, %rbp\n"
          "\tsubq\t$16, %rsp\n"
          "\tmovl\t%edi, -4(%rbp)\n"
          "\tmovl\t-4(%rbp), %eax\n"
          "\tmovl\t%eax, %esi\n"
          "\tleaq\t.LC0(%rip), %rdi\n"
          "\tmovl\t$0, %eax\n"
          "\tcall\tprintf@PLT\n"
          "\tnop\n"
          "\tleave\n"
          "\tret\n"
          "\n"
          "\t.globl\tmain\n"
          "\t.type\tmain, @function\n"
          "main:\n"
          "\tpushq\t%rbp\n"
          "\tmovq\t%rsp, %rbp\n",
          OUT_FILE);
}

void cgpostamble() {
    fputs("\tmovl\t$0, %eax\n"
          "\tpopq\t%rbp\n"
          "\tret\n",
          OUT_FILE);
}

int cgload(int value) {
    int reg = alloc_register();

    fprintf(OUT_FILE, "\tmovq\t$%d, %s\n", value, reglist[reg]);
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
