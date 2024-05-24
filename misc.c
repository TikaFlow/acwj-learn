//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void match(int token_type, char *what) {
    if (TOKEN.token_type == token_type) {
        scan();
        return;
    }
    fatals("expected", what);
}

// Print out fatal messages
void fatal(char *s) {
    fprintf(stderr, "%s on line %d.\n", s, LINE);
    exit(1);
}

void fatals(char *s1, char *s2) {
    fprintf(stderr, "%s:%s on line %d.\n", s1, s2, LINE);
    exit(1);
}

void fatald(char *s, int d) {
    fprintf(stderr, "%s:%d on line %d.\n", s, d, LINE);
    exit(1);
}

void fatalc(char *s, int c) {
    fprintf(stderr, "%s:%c on line %d.\n", s, c, LINE);
    exit(1);
}
