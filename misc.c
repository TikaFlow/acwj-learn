//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void match(int tokentype, char *what) {
    if (TOKEN.token == tokentype) {
        scan(&TOKEN);
        return;
    }
    printf("'%s' expected on line %d\n", what, LINE);
    exit(1);
}

void semi() {
    match(T_SEMI, ";");
}

void ident() {
    match(T_IDENT, "identifier");
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
