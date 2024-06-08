//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

// token name
static char *TOKEN_NAMES[] = {
        "EOF",
        // binary operators
        "=", "+=", "-=", "*=", "/=",
        "||", "&&", "|", "^", "&",
        "==", "!=", "<", ">", "<=", ">=",
        "<<", ">>", "+", "-", "*", "/",
        // unary operators
        "++", "--", "~", "!",
        // types
        "void", "char", "int", "long",
        // keywords
        "if", "else", "while", "for", "return",
        "struct", "union", "enum", "typedef", "extern",
        "break", "continue", "switch", "case", "default",
        // structures
        "int literal", "string literal", ";", "identifier",
        "{", "}", "(", ")", "[", "]",
        ",", ".", "->", ":",
};

// op name
char *OP_NAMES[] = {
        "NONE",
        "ASSIGN", "ADDASSIGN", "SUBASSIGN", "MULASSIGN", "DIVASSIGN",
        "LOGOR", "LOGAND", "OR", "XOR", "AND",
        "EQ", "NE", "LT", "GT", "LE", "GE",
        "LSHIFT", "RSHIFT",
        "ADD", "SUBTRACT", "MULTIPLY", "DIVIDE",
        "INTLIT", "STRLIT", "IDENT", "GLUE",
        "IF", "WHILE", "FUNCTION", "WIDEN", "RETURN", "FUNCCALL",
        "DEREF", "ADDR", "SCALE",
        "PREINC", "PREDEC", "POSTINC", "POSTDEC", "NEGATE", "INVERT",
        "LOGNOT", "TOBOOL",
        "BREAK", "CONTINUE", "SWITCH", "CASE", "DEFAULT",
};

// ptpye name
char *PTYPE_NAMES[] = {
        "NONE", "VOID", "CHAR", "INT", "LONG", "STRUCT", "UNION"
};

char *get_name(int value_type, int value) {
    int div, mod, len = 0;
    char *desc = "multiple pointers to STRUCT....";
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
                    len = snprintf(desc, 0x20 - len, "%s", "multiple ");
                }
                len = snprintf(desc, 0x20 - len, "%s", "pointers to ");
            }
            snprintf(desc, 0x20 - len, "%s", PTYPE_NAMES[div]);
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
