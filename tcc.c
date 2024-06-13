//
// Created by tika on 24-5-19.
//

#define EXTERN_

#include "data.h"

#undef EXTERN_

#include "decl.h"

struct macro {
    char *name;
    struct macro *next;
};

static struct macro *macro_head = NULL;
static struct macro *macro_tail = NULL;

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
    printf("Usage: %s [-vcTSM] [-o outfile] file [file ...]\n", prog);
    printf("\t-v give verbose output of the compilation stages\n");
    printf("\t-c generate object files but don't link them\n");
    printf("\t-T dump the AST trees for each input file\n");
    printf("\t-S generate assembly files but don't link them\n");
    printf("\t-M dump the symbol table for each input file\n");
    printf("\t-o outfile, produce the outfile executable file\n");
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
    char cmd[MAX_TEXT];
    struct macro *m;
    int len = 0;
    IN_FILE_NAME = file;
    OUT_FILE_NAME = alter_suffix(IN_FILE_NAME, 's');

    len += snprintf(cmd + len, MAX_TEXT - len, "%s %s", CPP_CMD, INC_DIR);
    // add macro to cmd
    for (m = macro_head; m; m = m->next) {
        len += snprintf(cmd + len, MAX_TEXT - len, " %s", m->name);
    }
    snprintf(cmd + len, MAX_TEXT - len, " %s", IN_FILE_NAME);

    if (!OUT_FILE_NAME) {
        fprintf(stderr, "Error: %s has no suffix, try .c on the end\n", IN_FILE_NAME);
        exit(1);
    }

    if (!(IN_FILE = popen(cmd, "r"))) {
        fprintf(stderr, "Unable to open %s: %s\n", IN_FILE_NAME, strerror(errno));
        exit(1);
    }

    if (!(OUT_FILE = fopen(OUT_FILE_NAME, "w"))) {
        fprintf(stderr, "Unable to create %s: %s\n", OUT_FILE_NAME, strerror(errno));
        exit(1);
    }

    init();
    if (FLAG_v) {
        printf("Compiling %s\n", file);
        printf("%s\n", cmd);
    }

    // start with scan the first token
    scan();
    gen_pre_amble(file);
    declare_global();
    gen_post_amble();

    if (FLAG_M) {
        printf("Symbols for %s\n", file);
        dump_sym_tables();
    }
    printf("Release static symbols for %s\n", file);
    reset_static_syms();

    fclose(OUT_FILE);
    fclose(IN_FILE);

    return OUT_FILE_NAME;
}

static char *do_assembly(char *file) {
    char cmd[MAX_TEXT];
    int err;

    char *o_file = alter_suffix(file, 'o');

    // as -o o_file file
    snprintf(cmd, MAX_TEXT, "%s %s %s", AS_CMD, o_file, file);

    if (FLAG_v) {
        printf("%s\n", cmd);
    }

    if ((err = system(cmd))) {
        fprintf(stderr, "Assembly of %s failed: %s\n", o_file, strerror(err));
    }

    return o_file;
}

static void do_link(char *o_file, char *obj_list[]) {
    int err, cnt = 0;
    char cmd[MAX_TEXT];

    cnt += snprintf(cmd + cnt, MAX_TEXT - cnt, "%s %s", LD_CMD, o_file);
    while (*obj_list) {
        cnt += snprintf(cmd + cnt, MAX_TEXT - cnt, " %s", *obj_list++);
    }
    snprintf(cmd + cnt, MAX_TEXT - cnt, " %s", LD_SUFFIX);

    if (FLAG_v) {
        printf("%s\n", cmd);
    }

    if ((err = system(cmd))) {
        fprintf(stderr, "Link failed: %s\n", strerror(err));
    }
}

/*
 * Parse the command line options and return the output file name.
 */
static char *parse_options(int argc, char *argv[], int *i_ptr) {
    int i;
    char *o_file = NULL;
    struct macro *mac;
    // parse the command line arguments
    for (i = 1; i < argc; i++) {
        // No leading '-', then it's a file name
        if (*argv[i] != '-') {
            break;
        }

        // each argument after '-' is a flag
        for (int j = 1; *argv[i] == '-' && argv[i][j]; j++) {
            switch (argv[i][j]) {
                case 'v':
                    FLAG_v = TRUE;
                    break;
                case 'c':
                    FLAG_c = TRUE;
                    break;
                case 'D':
                    if (argv[i][j + 1]) {
                        mac = malloc(sizeof(struct macro));
                        mac->name = strdup(argv[i]);
                        mac->next = NULL;

                        // add to macro list
                        if (macro_head) {
                            macro_tail->next = mac;
                            macro_tail = mac;
                        } else {
                            macro_head = macro_tail = mac;
                        }

                        // skip the macro name
                        while (argv[i][++j]);
                        j--;
                        break;
                    }
                    fatal("-D option must have a macro name after it");
                case 'T':
                    FLAG_T = TRUE;
                    break;
                case 'S':
                    FLAG_S = TRUE;
                    break;
                case 'M':
                    FLAG_M = TRUE;
                    break;
                case 'o':
                    // -o option must have a file name after it
                    if (argv[i][j + 1] == '\0' && i + 1 < argc) {
                        o_file = argv[++i];
                        break;
                    }
                    fprintf(stderr, "Expected file name after -o\n");
                default:
                    usage(argv[0]);
                    break;
            }
        }
    }

    *i_ptr = i;
    return o_file;
}

int main(int argc, char *argv[]) {

    if (argc < 2) usage(argv[0]);

    FLAG_v = FALSE;
    FLAG_c = FALSE;
    FLAG_T = FALSE;
    FLAG_S = FALSE;
    FLAG_M = FALSE;

    char *asm_file, *obj_file, *o_file;
    char *obj_list[MAX_OBJ];
    int i, obj_cnt = 0;

    // parse the options and get the output file name
    if (!(o_file = parse_options(argc, argv, &i))) {
        o_file = A_OUT;
    }

    while (i < argc) {
        // compile
        asm_file = do_compile(argv[i++]);

        if (FLAG_c || !FLAG_S) {
            // assembly
            obj_file = do_assembly(asm_file);

            if (obj_cnt >= MAX_OBJ - 1) {
                fprintf(stderr, "Too many object files\n");
                exit(1);
            }

            obj_list[obj_cnt++] = obj_file;
            obj_list[obj_cnt] = NULL;
        }

        if (!FLAG_S) {
            unlink(asm_file);
        }
    }

    // link
    if (!(FLAG_c || FLAG_S)) {
        do_link(o_file, obj_list);

        // delete the object files
        if (!FLAG_c) {
            for (i = 0; obj_list[i]; i++) {
                unlink(obj_list[i]);
            }
        }
    }

    return 0;
}
