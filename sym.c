//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static void add_sym(Symbol **head, Symbol **tail, Symbol *sym);

void reset_global_syms() {
    GLOBAL_HEAD = GLOBAL_TAIL = NULL;
    COMPOSITE_HEAD = COMPOSITE_TAIL = NULL;
}

void reset_local_syms() {
    LOCAL_HEAD = LOCAL_TAIL = NULL;
    PARAM_HEAD = PARAM_TAIL = NULL;
    FUNC_PTR = NULL;
}

void reset_sym_table() {
    reset_global_syms();
    reset_local_syms();
}

static Symbol *new_sym(char *name, int ptype, int stype, int class, int size, int posn) {
    Symbol *sym = (Symbol *) malloc(sizeof(Symbol));

    if (!sym) {
        fatal("Unable to malloc symbol in new_sym()");
    }

    sym->name = strdup(name);
    sym->ptype = ptype;
    sym->stype = stype;
    sym->class = class;
    sym->size = size;
    sym->posn = posn;
    sym->next = NULL;
    sym->first = NULL;

    // generate space if is a GLOBAL symbol
    if (class == C_GLOBAL) {
        gen_new_sym(sym);
    }

    return sym;
}

Symbol *add_global_sym(char *name, int ptype, int stype, int class, int size) {
    Symbol *sym = new_sym(name, ptype, stype, class, size, 0);
    add_sym(&GLOBAL_HEAD, &GLOBAL_TAIL, sym);

    return sym;
}

Symbol *add_param_sym(char *name, int ptype, int stype, int class, int size) {
    Symbol *sym = new_sym(name, ptype, stype, class, size, 0);
    add_sym(&PARAM_HEAD, &PARAM_TAIL, sym);

    return sym;
}

Symbol *add_local_sym(char *name, int ptype, int stype, int class, int size) {
    Symbol *sym = new_sym(name, ptype, stype, class, size, 0);
    add_sym(&LOCAL_HEAD, &LOCAL_TAIL, sym);

    return sym;
}

static void add_sym(Symbol **head, Symbol **tail, Symbol *sym) {
    if (!(head && tail && sym)) {
        fatal("Either head or tail or sym is NULL in add_sym()");
    }

    if (*tail) {
        (*tail)->next = sym;
        *tail = sym;
    } else {
        *head = *tail = sym;
    }

    sym->next = NULL;
}

static Symbol *find_sym_in_list(char *s, Symbol *list) {
    while (list) {
        if ((list->name) && *(list->name) == *s && !strcmp(list->name, s)) {
            return list;
        }

        list = list->next;
    }

    return NULL;
}

Symbol *find_global(char *s) {
    return find_sym_in_list(s, GLOBAL_HEAD);
}

static Symbol *find_param(char *s) {
    if (FUNC_PTR) {
        return find_sym_in_list(s, FUNC_PTR->first);
    }

    return NULL;
}

Symbol *find_local(char *s) {
    Symbol *sym;

    return (sym = find_param(s)) ? sym : find_sym_in_list(s, LOCAL_HEAD);
}

Symbol *find_composite(char *s) {
    return find_sym_in_list(s, COMPOSITE_HEAD);
}

Symbol *find_sym(char *s) {
    Symbol *sym;

    return (sym = find_local(s)) ? sym : find_global(s);
}
