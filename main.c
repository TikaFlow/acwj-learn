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
    TOKEN_BACK.token_type = T_EOF;
    TOKEN_BACK.int_value = 0;
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

    // use C print_int
    add_sym("printd", P_VOID, S_FUNCTION, 0, 0);
    add_sym("printc", P_VOID, S_FUNCTION, 0, 0);
    add_sym("prints", P_VOID, S_FUNCTION, 0, 0);

    // start with scan the first token
    scan();

    gen_pre_amble();
    declare_global();
    gen_post_amble();

    fclose(OUT_FILE);
    fclose(IN_FILE);
    exit(0);
}
