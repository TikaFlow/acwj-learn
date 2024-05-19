//
// Created by tika on 24-5-19.
//

#include "defs.h"

#define extern_

#include "data.h"

#undef extern_

#include "decl.h"
#include <errno.h>

static void init() {
    Line = 1;
    Putback = '\n';
}

static void usage(char *prog) {
    fprintf(stderr, "Usage: %s infile \n", prog);
    exit(1);
}

char *tkstr[] = {"+", "-", "*", "/", "intlit"};

static void scanfile() {
    Token T;

    while (scan(&T)) {
        printf("Token %s", tkstr[T.token]);
        if (T.token == T_INTLIT)
            printf(", value %d", T.intvalue);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) usage(argv[0]);

    init();

    if ((Infile = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    scanfile();
    exit(0);
}
