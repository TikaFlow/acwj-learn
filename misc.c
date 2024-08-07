//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

// token name
static char *TOKEN_NAMES[] = {
        "EOF",
        // binary operators
        "=", "+=", "-=", "*=", "/=", "%=",
        "?", "||", "&&", "|", "^", "&",
        "==", "!=", "<", ">", "<=", ">=",
        "<<", ">>", "+", "-", "*", "/", "%",
        // unary operators
        "++", "--", "~", "!",
        // types
        "void", "char", "short", "int", "long",
        // keywords
        "if", "else", "while", "for", "return", "sizeof",
        "struct", "union", "enum", "typedef", "extern",
        "break", "continue", "switch", "case", "default",
        "static",
        // structures
        "int literal", "string literal", ";", "identifier",
        "{", "}", "(", ")", "[", "]",
        ",", ".", "->", ":",
};

// op name
char *OP_NAMES[] = {
        "NONE",
        "ASSIGN", "ADDASSIGN", "SUBASSIGN", "MULASSIGN", "DIVASSIGN", "MODASSIGN",
        "TERNARY", "LOGOR", "LOGAND", "OR", "XOR", "AND",
        "EQ", "NE", "LT", "GT", "LE", "GE",
        "LSHIFT", "RSHIFT", "ADD", "SUBTRACT", "MULTIPLY", "DIVIDE", "MOD",
        "INTLIT", "STRLIT", "IDENT", "GLUE",
        "IF", "WHILE", "FUNCTION", "WIDEN", "RETURN", "FUNCCALL",
        "DEREF", "ADDR", "SCALE",
        "PREINC", "PREDEC", "POSTINC", "POSTDEC", "NEGATE", "INVERT",
        "LOGNOT", "TOBOOL",
        "BREAK", "CONTINUE", "SWITCH", "CASE", "DEFAULT", "CAST",
        "NOP", "DECLARE",
};

// ptpye name
char *PTYPE_NAMES[] = {
        "NONE", "VOID", "CHAR", "SHORT", "INT", "LONG", "STRUCT", "UNION"
};

char *get_name(int value_type, int value) {
    int div, mod, len = 0, desc_len = 0x20;
    char *desc = calloc(desc_len, sizeof(char));
    switch (value_type) {
        case V_TOKEN:
            return TOKEN_NAMES[value];
        case V_OP:
            return OP_NAMES[value];
        case V_PTYPE:
            div = value / 0x10;
            mod = value % 0x10;
            if (mod) {
                if (mod > 1) {
                    len = snprintf(desc, desc_len - len, "%s", "multiple ");
                }
                len = snprintf(desc, desc_len - len, "%s", "pointers to ");
            }
            snprintf(desc, desc_len - len, "%s", PTYPE_NAMES[div]);
            return desc;
        default:
            return "UNKNOWN";
    }
}

void match(int token_type, char *what) {
    if (TOKEN.token_type == token_type) {
        scan();
        return;
    }
    fatals("expected", what);
}

void debug(char *s) {
    fprintf(stdout, "[DEBUG] %s:%d %s.\n", IN_FILE_NAME, LINE, s);
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

char *format_str(char *fmt, ...) {
    char *str = calloc(MAX_TEXT, sizeof(char));
    if (!str) {
        fprintf(stderr, "Out of memory.");
    }

    va_list args;
    va_start(args, fmt);
    vsprintf(str, fmt, args);
    va_end(args);

    return str;
}
