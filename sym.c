//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

// next free GLOBAL symbol slot
static int GLOBAL_TOP = 0;
// next free LOCAL symbol slot
static int LOCAL_TOP = SYM_TAB_LEN - 1;

static void update_sym(int slot, char *name, int ptype, int stype, int class, int end_label, int size, int posn);

static int new_global() {
    int pos;
    if ((pos = GLOBAL_TOP++) >= LOCAL_TOP) {
        fatal("too many global symbols!");
    }
    return pos;
}

static int new_local() {
    int pos;
    if ((pos = LOCAL_TOP--) <= GLOBAL_TOP) {
        fatal("too many local symbols!");
    }
    return pos;
}

static int find_global(char *s) {
    for (int index = 0; index < GLOBAL_TOP; index++) {
        if (*s == *SYM_TAB[index].name && !strcmp(s, SYM_TAB[index].name)) {
            return index;
        }
    }
    return NOT_FOUND;
}

static int find_local(char *s) {
    for (int index = SYM_TAB_LEN - 1; index > LOCAL_TOP; index--) {
        if (*s == *SYM_TAB[index].name && !strcmp(s, SYM_TAB[index].name)) {
            return index;
        }
    }
    return NOT_FOUND;
}

int add_global_sym(char *name, int ptype, int stype, int end_label, int size) {
    int slot;

    if ((slot = find_global(name)) != NOT_FOUND) {
        return slot;
    }

    slot = new_global();
    update_sym(slot, name, ptype, stype, C_GLOBAL, end_label, size, 0);
    gen_new_sym(slot);
    return slot;
}

int add_local_sym(char *name, int ptype, int stype, int end_label, int size) {
    int slot, posn;

    if ((slot = find_local(name)) != NOT_FOUND) {
        return slot;
    }

    slot = new_local();
    posn = gen_get_local_offset(ptype, FALSE);
    update_sym(slot, name, ptype, stype, C_LOCAL, end_label, size, posn);
    return slot;
}

static void update_sym(int slot, char *name, int ptype, int stype, int class, int end_label, int size, int posn) {
    if (slot == NOT_FOUND || slot >= SYM_TAB_LEN) {
        fatal("Invalid symbol slot number in updatesym()");
    }
    SYM_TAB[slot].name = strdup(name);
    SYM_TAB[slot].ptype = ptype;
    SYM_TAB[slot].stype = stype;
    SYM_TAB[slot].class = class;
    SYM_TAB[slot].end_label = end_label;
    SYM_TAB[slot].size = size;
    SYM_TAB[slot].posn = posn;
}

int find_sym(char *s) {
    int slot;
    return (slot = find_local(s)) == NOT_FOUND ? find_global(s) : slot;
}
