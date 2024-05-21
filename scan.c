//
// Created by tika on 24-5-19.
//

#include "data.h"
#include "decl.h"

static int chrpos(char *s, int c) {
    char *p;

    p = strchr(s, c);
    return p ? p - s : -1;
}

static int next() {
    int c;

    if (PUT_BACK) {
        c = PUT_BACK;
        PUT_BACK = 0;
        return c;
    }

    c = fgetc(IN_FILE);
    if (c == '\n')
        LINE++;

    return c;
}

static void putback(int c) {
    PUT_BACK = c;
}

static int skip() {
    int c;

    while ((c = next()) == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');

    return c;
}

static int scanint(int c) {
    int k, val = 0;

    while ((k = chrpos("0123456789", c)) >= 0) {
        val = val * 10 + k;
        c = next();
    }

    putback(c);
    return val;
}

static int scanident(int c, char *buf, int lim) {
    int len = 0;
    while (isalpha(c) || isdigit(c) || c == '_') {
        if (len >= lim - 1) {
            printf("Identifier too long on line %d\n", LINE);
            exit(1);
        }
        buf[len++] = c;
        c = next();
    }
    putback(c);
    buf[len] = '\0';
    return len;
}

static int keyword(char *s) {
    switch (*s) {
        case 'p':
            if (!strcmp(s, "print")) {
                return T_PRINT;
            }
            break;
        case 'i':
            if (!strcmp(s, "int")) {
                return T_INT;
            }
            break;
    }
    return 0;
}

int scan(Token *t) {
    int c = skip(), tokentype = 0;

    switch (c) {
        case EOF:
            t->token = T_EOF;
            return 0;
        case '+':
            t->token = T_PLUS;
            break;
        case '-':
            t->token = T_MINUS;
            break;
        case '*':
            t->token = T_STAR;
            break;
        case '/':
            t->token = T_SLASH;
            break;
        case ';':
            t->token = T_SEMI;
            break;
        case '=':
            t->token = T_EQUALS;
            break;
        default:
            if (isdigit(c)) {
                t->intvalue = scanint(c);
                t->token = T_INTLIT;
                break;
            }
            if (isalpha(c) || c == '_') {
                scanident(c, TEXT, TEXT_LEN);

                if ((tokentype = keyword(TEXT))) {
                    t->token = tokentype;
                    break;
                }

                t->token = T_IDENT;
                break;
            }
            printf("Unrecognized character: %c on line %d\n", c, LINE);
            exit(1);
    }
    return 1;
}
