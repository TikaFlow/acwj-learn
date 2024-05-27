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

static char scan_char() {
    int c = next();

    if (c == '\\') {
        switch (c = next()) {
            case 'a':
                return '\a';
            case 'b':
                return '\b';
            case 'f':
                return '\f';
            case 'n':
                return '\n';
            case 'r':
                return '\r';
            case 't':
                return '\t';
            case 'v':
                return '\v';
            case '"':
                return '\"';
            case '\'':
                return '\'';
            case '\\':
                return '\\';
            default:
                fatalc("Invalid escape sequence \\", c);
        }
    }

    return c;
}

static long scan_int(int c) {
    int k;
    long val = 0;

    while ((k = char_pos("0123456789", c)) >= 0) {
        val = val * 10 + k;
        c = next();
    }

    put_back(c);
    return val;
}

static int scan_string() {
    int i;
    char c;

    for (i = 0; i < TEXT_LEN - 1; i++) {
        if ((c = scan_char()) == '\"') {
            TEXT[i] = '\0';
            return i;
        }
        TEXT[i] = c;
    }

    fatal("String is too long");
    return 0;
}

static int scan_ident(int c) {
    int len = 0;
    while (isalpha(c) || isdigit(c) || c == '_') {
        if (len >= TEXT_LEN - 1) {
            fatal("Identifier too long");
        }
        TEXT[len++] = c;
        c = next();
    }
    put_back(c);
    TEXT[len] = '\0';
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
        case 'l':
            if (!strcmp(s, "long")) {
                return T_LONG;
            }
            break;
        case 'r':
            if (!strcmp(s, "return")) {
                return T_RETURN;
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

void reject_token() {
    if (TOKEN_BACK.token_type) {
        fatal("Can't reject token twice");
    }
    TOKEN_BACK = TOKEN;
}

int scan() {
    if (TOKEN_BACK.token_type) {
        // TOKEN = TOKEN_BACK;
        // reset token_back
        TOKEN_BACK.token_type = T_EOF;
        return TRUE;
    }

    int c = skip(), token_type;

    switch (c) {
        case EOF:
            TOKEN.token_type = T_EOF;
            return 0;
        case '+':
            TOKEN.token_type = T_PLUS;
            break;
        case '-':
            TOKEN.token_type = T_MINUS;
            break;
        case '*':
            TOKEN.token_type = T_STAR;
            break;
        case '/':
            TOKEN.token_type = T_SLASH;
            break;
        case ';':
            TOKEN.token_type = T_SEMI;
            break;
        case '{':
            TOKEN.token_type = T_LBRACE;
            break;
        case '}':
            TOKEN.token_type = T_RBRACE;
            break;
        case '[':
            TOKEN.token_type = T_LBRACKET;
            break;
        case ']':
            TOKEN.token_type = T_RBRACKET;
            break;
        case '(':
            TOKEN.token_type = T_LPAREN;
            break;
        case ')':
            TOKEN.token_type = T_RPAREN;
            break;
        case ',':
            TOKEN.token_type = T_COMMA;
            break;
        case '=':
            if ((c = next()) == '=') {
                TOKEN.token_type = T_EQ;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_ASSIGN;
            break;
        case '!':
            if ((c = next()) == '=') {
                TOKEN.token_type = T_NE;
                break;
            }
            put_back(c);
            fatalc("Unrecognized character", c);
            break;
        case '<':
            if ((c = next()) == '=') {
                TOKEN.token_type = T_LE;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_LT;
            break;
        case '>':
            if ((c = next()) == '=') {
                TOKEN.token_type = T_GE;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_GT;
            break;
        case '&':
            if ((c = next()) == '&') {
                TOKEN.token_type = T_LOGAND;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_AMPER;
            break;
        case '\'':
            TOKEN.int_value = scan_char();
            TOKEN.token_type = T_INTLIT;
            if (next() != '\'') {
                fatal("Unclosed character literal");
            }
            break;
        case '"':
            scan_string();
            TOKEN.token_type = T_STRLIT;
            break;
        default:
            if (isdigit(c)) {
                TOKEN.int_value = scan_int(c);
                TOKEN.token_type = T_INTLIT;
                break;
            }
            if (isalpha(c) || c == '_') {
                scan_ident(c);

                if ((token_type = keyword(TEXT))) {
                    TOKEN.token_type = token_type;
                    break;
                }

                TOKEN.token_type = T_IDENT;
                break;
            }
            fatalc("Unrecognized character", c);
    }
    return TRUE;
}
