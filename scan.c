//
// Created by tika on 24-5-19.
//

#include "data.h"
#include "decl.h"

static int char_pos(char *s, int c) {
    char *p;

    p = strchr(s, c);
    return p ? (int) (p - s) : NOT_FOUND;
}

static int next() {
    int c, l;

    if (PUT_BACK) {
        c = PUT_BACK;
        PUT_BACK = 0;
        return c;
    }

    c = fgetc(IN_FILE);

    // pre-processor statements
    while (c == '#') {
        // skip #
        scan();

        l = (int) TOKEN.int_value;
        // skip line number
        match(T_INTLIT, "pre-processor line number");

        // file name
        if (TOKEN.token_type != T_STRLIT) {
            fatal("Expected file name");
        }
        if (TEXT[0] != '<') { // a real file name
            if (strcmp(TEXT, IN_FILE_NAME)) {
                IN_FILE_NAME = strdup(TEXT);
            }

            LINE = l;
        }

        // skip to next line
        while ((c = fgetc(IN_FILE)) != '\n' && c != EOF);
        if (c == EOF) {
            return c;
        }
        c = fgetc(IN_FILE);
    }

    if (c == '\n') {
        LINE++;
    }
    return c;
}

static void put_back(int c) {
    PUT_BACK = c;
}

static int skip_line_comment() {
    int c;
    while ((c = next()) != '\n' && c != EOF);

    return c;
}

static int skip_block_comment() {
    int closed = FALSE, c;

    while ((c = next()) != EOF) {
        if (c == '*') {
            if ((c = next()) == '/') {
                c = next();
                closed = TRUE;
                break;
            } else {
                put_back(c);
            }
        }
    }

    if (!closed) {
        fatal("Unterminated comment");
    }

    return c;
}

static int skip() {
    int c = next();

    while (TRUE) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
            c = next();
        } else if (c == '/') {
            int n = next();

            switch (n) {
                case '/':
                    c = skip_line_comment();
                    break;
                case '*':
                    c = skip_block_comment();
                    break;
                default:
                    put_back(n);
                    return c;
            }
        } else {
            break;
        }
    }

    return c;
}

static int hex_char() {
    int c, i, val = 0, seen = FALSE;

    while (isxdigit(c = next())) {
        seen = TRUE;
        i = char_pos("0123456789abcdef", tolower(c));
        val = (val << 4) + i;
    }

    put_back(c);
    if (!seen) {
        fatal("missing hex digit after \\x");
    }
    if (val > 255) {
        fatal("hex escape value out of range");
    }

    return val;
}

static int scan_char(char *esc) {
    int i, oct, c = next();

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
                if (esc) {
                    *esc = '\\';
                }
                return '\"';
            case '\'':
                return '\'';
            case '\\':
                return '\\';
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                for (oct = i = 0; c >= '0' && c <= '7'; c = next()) {
                    if (++i > 3) {
                        break;
                    }
                    oct = oct * 8 + (c - '0');
                }
                put_back(c);
                return (char) oct;
            case 'x':
                return hex_char();
            default:
                fatalc("Invalid escape sequence \\", c);
        }
    }

    return c;
}

static long scan_int(int c) {
    int k, radix = 10;
    long val = 0;

    if (c == '0') {
        c = next();
        if (c == 'x' || c == 'X') { // hexadecimal
            c = next();
            radix = 16;
        } else if (c == 'b' || c == 'B') { // binary
            c = next();
            radix = 2;
        } else { // octal
            radix = 8;
        }
    }

    while ((k = char_pos("0123456789abcdef", tolower(c))) >= 0) {
        if (k >= radix) {
            fatald("Invalid digit of radix", radix);
        }

        val = val * radix + k;
        c = next();
    }

    put_back(c);
    return val;
}

static int scan_string() {
    int i, c;
    char esc = 0;

    for (i = 0; i < MAX_TEXT - 1; i++) {
        if ((c = scan_char(&esc)) == '\"') {
            if (!esc) {
                TEXT[i] = '\0';
                return i;
            }
            esc = 0;
        }
        TEXT[i] = c;
    }

    fatal("String is too long");
    return 0;
}

static int scan_ident(int c) {
    int len = 0;
    while (isalpha(c) || isdigit(c) || c == '_') {
        if (len >= MAX_TEXT - 1) {
            fatal("Identifier too long");
        }
        TEXT[len++] = (char) c;
        c = next();
    }
    put_back(c);
    TEXT[len] = '\0';
    return len;
}

static int keyword(char *s) {
    switch (*s) {
        case 'b':
            if (!strcmp(s, "break")) {
                return T_BREAK;
            }
            break;
        case 'c':
            if (!strcmp(s, "case")) {
                return T_CASE;
            }
            if (!strcmp(s, "char")) {
                return T_CHAR;
            }
            if (!strcmp(s, "continue")) {
                return T_CONTINUE;
            }
            break;
        case 'd':
            if (!strcmp(s, "default")) {
                return T_DEFAULT;
            }
            break;
        case 'e':
            if (!strcmp(s, "else")) {
                return T_ELSE;
            }
            if (!strcmp(s, "enum")) {
                return T_ENUM;
            }
            if (!strcmp(s, "extern")) {
                return T_EXTERN;
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
        case 's':
            if (!strcmp(s, "sizeof")) {
                return T_SIZEOF;
            }
            if (!strcmp(s, "short")) {
                return T_SHORT;
            }
            if (!strcmp(s, "struct")) {
                return T_STRUCT;
            }
            if (!strcmp(s, "switch")) {
                return T_SWITCH;
            }
            break;
        case 't':
            if (!strcmp(s, "typedef")) {
                return T_TYPEDEF;
            }
            break;
        case 'u':
            if (!strcmp(s, "union")) {
                return T_UNION;
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

Token peek_token() {
    if (TOKEN_BACK.token_type) {
        return TOKEN_BACK;
    }

    Token t_tk;
    char t_text[MAX_TEXT + 1];

    // store current one
    t_tk = TOKEN;
    memcpy(t_text, TEXT, sizeof(TEXT));
    // get next one
    scan();
    TOKEN_BACK = TOKEN;
    memcpy(TEXT_BACK, TEXT, sizeof(TEXT));
    // restore current one
    TOKEN = t_tk;
    memcpy(TEXT, t_text, sizeof(TEXT));

    return TOKEN_BACK;
}

int scan() {
    if (TOKEN_BACK.token_type) {
        TOKEN = TOKEN_BACK;
        memcpy(TEXT, TEXT_BACK, sizeof(TEXT));
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
            if ((c = next()) == '+') {
                TOKEN.token_type = T_INC;
                break;
            } else if (c == '=') {
                TOKEN.token_type = T_ASPLUS;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_PLUS;
            break;
        case '-':
            if ((c = next()) == '-') {
                TOKEN.token_type = T_DEC;
                break;
            } else if (c == '>') {
                TOKEN.token_type = T_ARROW;
                break;
            } else if (c == '=') {
                TOKEN.token_type = T_ASMINUS;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_MINUS;
            break;
        case '*':
            if ((c = next()) == '=') {
                TOKEN.token_type = T_ASSTAR;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_STAR;
            break;
        case '/':
            if ((c = next()) == '=') {
                TOKEN.token_type = T_ASSLASH;
                break;
            }
            put_back(c);
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
        case '.':
            TOKEN.token_type = T_DOT;
            break;
        case ':':
            TOKEN.token_type = T_COLON;
            break;
        case '~':
            TOKEN.token_type = T_INVERT;
            break;
        case '^':
            TOKEN.token_type = T_XOR;
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
            TOKEN.token_type = T_LOGNOT;
            break;
        case '<':
            if ((c = next()) == '=') {
                TOKEN.token_type = T_LE;
                break;
            }
            if (c == '<') {
                TOKEN.token_type = T_LSHIFT;
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
            if (c == '>') {
                TOKEN.token_type = T_RSHIFT;
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
            TOKEN.token_type = T_AND;
            break;
        case '|':
            if ((c = next()) == '|') {
                TOKEN.token_type = T_LOGOR;
                break;
            }
            put_back(c);
            TOKEN.token_type = T_OR;
            break;
        case '\'':
            TOKEN.int_value = scan_char(NULL);
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
                // TODO postfix of integer?
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
