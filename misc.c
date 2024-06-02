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

void warning(char *s) {
    fprintf(stderr, "[WARNING] %s:%d %s.\n", IN_FILE_NAME, LINE, s);
}

// Print out fatal messages
void fatal(char *s) {
    fprintf(stderr, "[ERROR] %s:%d %s.\n", IN_FILE_NAME, LINE, s);
    fclose(OUT_FILE);
    fclose(IN_FILE);
    unlink(OUT_FILE_NAME);
    exit(1);
}

void fatals(char *s1, char *s2) {
    fprintf(stderr, "[ERROR] %s:%d %s:%s.\n", IN_FILE_NAME, LINE, s1, s2);
    fclose(OUT_FILE);
    fclose(IN_FILE);
    unlink(OUT_FILE_NAME);
    exit(1);
}

void fatald(char *s, int d) {
    fprintf(stderr, "[ERROR] %s:%d %s:%d.\n", IN_FILE_NAME, LINE, s, d);
    fclose(OUT_FILE);
    fclose(IN_FILE);
    unlink(OUT_FILE_NAME);
    exit(1);
}

void fatalc(char *s, int c) {
    fprintf(stderr, "[ERROR] %s:%d %s:%c.\n", IN_FILE_NAME, LINE, s, c);
    fclose(OUT_FILE);
    fclose(IN_FILE);
    unlink(OUT_FILE_NAME);
    exit(1);
}
