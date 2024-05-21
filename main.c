//
// Created by tika on 24-5-19.
//

#include <errno.h>

#define extern_

#include "data.h"

#undef extern_

#include "decl.h"

static void init() {
    LINE = 1;
    PUT_BACK = '\n';
}

static void usage(char *prog) {
    fprintf(stderr, "Usage: %s infile\n", prog);
    exit(1);
}

int main(int argc, char *argv[]) {

    if (argc != 2) usage(argv[0]);

    init();

    if ((IN_FILE = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    if ((OUT_FILE = fopen("out.s", "w")) == NULL) {
        fprintf(stderr, "Unable to create out.s: %s\n", strerror(errno));
        exit(1);
    }

    scan(&TOKEN);
    genpreamble();
    statements();
    genpostamble();

    fclose(OUT_FILE);
    exit(0);
}
