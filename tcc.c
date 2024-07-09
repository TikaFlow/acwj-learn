//
// Created by tika on 24-5-19.
//

#define EXTERN_

#include "data.h"

#undef EXTERN_

#include "decl.h"

static void init() {
    LINE = 1;
    PUT_BACK = '\n';
    FLAG_LINE_START = TRUE;
    TOKEN_BACK.token_type = T_EOF;
    TOKEN_BACK.int_value = 0;

    reset_sym_table();
    reset_global_syms();
    reset_local_syms();
}

static void usage(char *prog) {
    printf("Usage: %s file\n", prog);
}

static char *alter_suffix(char *str, char suf) {
    char *new_str, *pos;

    if (!(new_str = strdup(str))) {
        return NULL;
    }

    if (!(pos = strrchr(new_str, '.'))) {
        return NULL;
    }

    if (!*++pos) {
        return NULL;
    }

    *pos++ = suf;
    *pos = 0;

    return new_str;
}

static char *do_compile(char *file) {
    IN_FILE_NAME = file;
    OUT_FILE_NAME = alter_suffix(IN_FILE_NAME, 's');

    if (!OUT_FILE_NAME) {
        fprintf(stderr, "Error: %s has no suffix, try .c on the end\n", IN_FILE_NAME);
        exit(1);
    }

    if (!(IN_FILE = fopen(IN_FILE_NAME, "r"))) {
        fprintf(stderr, "Unable to open %s: %s\n", IN_FILE_NAME, strerror(errno));
        exit(1);
    }

    if (!(OUT_FILE = fopen(OUT_FILE_NAME, "w"))) {
        fprintf(stderr, "Unable to create %s: %s\n", OUT_FILE_NAME, strerror(errno));
        exit(1);
    }

    init();
    printf("Compiling %s\n", file);

    // start with scan the first token
    scan();
    gen_pre_amble(file);
    declare_global();
    gen_post_amble();

#ifdef DEBUG
    printf("Symbols for %s\n", file);
    dump_sym_tables();
    printf("Release static symbols for %s\n", file);
    reset_static_syms();
#endif // DEBUG

    fclose(OUT_FILE);
    fclose(IN_FILE);

    return OUT_FILE_NAME;
}

int main(int argc, char *argv[]) {

    if (argc < 2) usage(argv[0]);

    // compile
    do_compile(argv[1]);

    return 0;
}
