//
// Created by tika on 24-5-19.
//

#include "data.h"
#include "decl.h"

static int char_pos(char *s, int c) {
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
    if (c == '\n') {
        LINE++;
    }
    return c;
}

static void put_back(int c) {
    PUT_BACK = c;
}

static int skip() {
    int c;

    while ((c = next()) == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');

    return c;
}

static int scan_int(int c) {
    int k, val = 0;

    while ((k = char_pos("0123456789", c)) >= 0) {
        val = val * 10 + k;
        c = next();
    }

    put_back(c);
    return val;
}

static int scan_ident(int c, char *buf) {
    int len = 0;
    while (isalpha(c) || isdigit(c) || c == '_') {
        if (len >= TEXT_LEN - 1) {
            fatal("Identifier too long");
        }
        buf[len++] = c;
        c = next();
    }
    put_back(c);
    buf[len] = '\0';
    return len;
}

static int keyword(char *s) {
    switch (*s) {
        case 'c':
            if (!strcmp(s, "char")) {
                return T_CHAR;
            }
            break;
        case 'e':
            if (!strcmp(s, "else")) {
                return T_ELSE;
            }
            break;
        case 'f':
            if (!strcmp(s, "for")) {
                return T_FOR;
            }
            break;
        case 'i':
            if (!strcmp(s, "if")) {
                return T_IF;
            }
            if (!strcmp(s, "int")) {
                return T_INT;
            }
            break;
        case 'p':
            if (!strcmp(s, "print")) {
                return T_PRINT;
            }
            break;
        case 'v':
            if (!strcmp(s, "void")) {
                return T_VOID;
            }
            break;
        case 'w':
            if (!strcmp(s, "while")) {
                return T_WHILE;
            }
            break;
    }
    return 0;
}

int scan(Token *t) {
    int c = skip(), tokentype;

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
        case '{':
            t->token = T_LBRACE;
            break;
        case '}':
            t->token = T_RBRACE;
            break;
        case '(':
            t->token = T_LPAREN;
            break;
        case ')':
            t->token = T_RPAREN;
            break;
        case '=':
            if ((c = next()) == '=') {
                t->token = T_EQ;
                break;
            }
            put_back(c);
            t->token = T_ASSIGN;
            break;
        case '!':
            if ((c = next()) == '=') {
                t->token = T_NE;
                break;
            }
            put_back(c);
            fatalc("Unrecognized character", c);
            break;
        case '<':
            if ((c = next()) == '=') {
                t->token = T_LE;
                break;
            }
            put_back(c);
            t->token = T_LT;
            break;
        case '>':
            if ((c = next()) == '=') {
                t->token = T_GE;
                break;
            }
            put_back(c);
            t->token = T_GT;
            break;
        default:
            if (isdigit(c)) {
                t->intvalue = scan_int(c);
                t->token = T_INTLIT;
                break;
            }
            if (isalpha(c) || c == '_') {
                scan_ident(c, TEXT);

                if ((tokentype = keyword(TEXT))) {
                    t->token = tokentype;
                    break;
                }

                t->token = T_IDENT;
                break;
            }
            fatalc("Unrecognized character", c);
    }
    return 1;
}
