//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

// next free symbol slot
static int SYM_TAB_TOP = 0;

int findglob(char *s) {
    for (int index = 0; index < SYM_TAB_TOP; index++) {
        if (*s == *SYM_TAB[index].name && !strcmp(s, SYM_TAB[index].name)) {
            return index;
        }
    }
    return -1;
}

static int newglob() {
    int pos;
    if ((pos = SYM_TAB_TOP++) >= SYM_TAB_LEN) {
        fatal("too many global symbols!");
    }
    return pos;
}

int addglob(char *name) {
    int pos;
    if ((pos = findglob(name)) >= 0) {
        return pos;
    }

    pos = newglob();
    SYM_TAB[pos].name = strdup(name);
    return pos;
}
