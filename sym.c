//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"


static void update_sym(int slot, char *name, int ptype, int stype, int class, int end_label, int size, int posn);

void reset_global_syms() {
    LOCAL_TOP = 0;
}

void reset_loccal_syms() {
    LOCAL_TOP = SYM_TAB_LEN - 1;
}

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
        if (SYM_TAB[index].class == C_PARAM) {
            continue;
        }
        if (*s == *SYM_TAB[index].name && !strcmp(s, SYM_TAB[index].name)) {
            return index;
        }
    }
    return NOT_FOUND;
}

static int find_local(char *s) {
    for (int index = LOCAL_TOP + 1; index < SYM_TAB_LEN; index++) {
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

int add_local_sym(char *name, int ptype, int stype, int is_param, int size) {
    if (find_local(name) != NOT_FOUND) {
        return -1;
    }

    int local_slot = new_local();
    if (is_param) {
        update_sym(local_slot, name, ptype, stype, C_PARAM, 0, size, 0);
        update_sym(new_global(), name, ptype, stype, C_PARAM, 0, size, 0);
    } else {
        update_sym(local_slot, name, ptype, stype, C_LOCAL, 0, size, 0);
    }

    return local_slot;
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
