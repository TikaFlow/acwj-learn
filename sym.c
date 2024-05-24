//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

// next free symbol slot
static int SYM_TAB_TOP = 0;

int find_sym(char *s) {
    for (int index = 0; index < SYM_TAB_TOP; index++) {
        if (*s == *SYM_TAB[index].name && !strcmp(s, SYM_TAB[index].name)) {
            return index;
        }
    }
    return -1;
}

static int new_glob() {
    int pos;
    if ((pos = SYM_TAB_TOP++) >= SYM_TAB_LEN) {
        fatal("too many global symbols!");
    }
    return pos;
}

int add_sym(char *name, int ptype, int stype, int end_label) {
    int pos;
    if ((pos = find_sym(name)) >= 0) {
        return pos;
    }

    pos = new_glob();
    SYM_TAB[pos].name = strdup(name);
    SYM_TAB[pos].ptype = ptype;
    SYM_TAB[pos].stype = stype;
    SYM_TAB[pos].end_label = end_label;
    return pos;
}
