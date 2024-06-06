//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static void add_sym(Symbol **head, Symbol **tail, Symbol *sym);

void reset_global_syms() {
    GLOBAL_HEAD = GLOBAL_TAIL = NULL;
    STRUCT_HEAD = STRUCT_TAIL = NULL;
    UNION_HEAD = UNION_TAIL = NULL;
    MEMBER_HEAD = MEMBER_TAIL = NULL;
    ENUM_HEAD = ENUM_TAIL = NULL;
    TYPEDEF_HEAD = TYPEDEF_TAIL = NULL;
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

static Symbol *new_sym(char *name, int ptype, Symbol *ctype, int stype, int class, int n_elem, int posn) {
    Symbol *sym = (Symbol *) malloc(sizeof(Symbol));

    if (!sym) {
        fatal("Unable to malloc symbol in new_sym()");
    }

    sym->name = strdup(name);
    sym->ptype = ptype;
    sym->ctype = ctype;
    sym->stype = stype;
    sym->class = class;
    sym->n_elem = n_elem;
    sym->posn = posn;
    sym->next = NULL;
    sym->first = NULL;
    sym->init_list = NULL;

    if (is_int(ptype) || is_ptr(ptype)) {
        sym->size = n_elem * size_of_type(ptype, ctype);
    }

    return sym;
}

Symbol *add_global_sym(char *name, int ptype, Symbol *ctype, int stype, int class, int n_elem, int posn) {
    Symbol *sym = new_sym(name, ptype, ctype, stype, class, n_elem, posn);
    if (ptype == P_STRUCT || ptype == P_UNION) {
        sym->size = ctype->size;
    }
    add_sym(&GLOBAL_HEAD, &GLOBAL_TAIL, sym);

    return sym;
}

Symbol *add_param_sym(char *name, int ptype, Symbol *ctype, int stype) {
    Symbol *sym = new_sym(name, ptype, ctype, stype, C_PARAM, 1, 0);
    add_sym(&PARAM_HEAD, &PARAM_TAIL, sym);

    return sym;
}

Symbol *add_local_sym(char *name, int ptype, Symbol *ctype, int stype, int n_elem) {
    Symbol *sym = new_sym(name, ptype, ctype, stype, C_LOCAL, n_elem, 0);
    if (ptype == P_STRUCT || ptype == P_UNION) {
        sym->size = ctype->size;
    }
    add_sym(&LOCAL_HEAD, &LOCAL_TAIL, sym);

    return sym;
}

Symbol *add_struct_sym(char *name) {
    Symbol *sym = new_sym(name, P_STRUCT, NULL, S_NONE, C_STRUCT, 0, 0);

    add_sym(&STRUCT_HEAD, &STRUCT_TAIL, sym);

    return sym;
}

Symbol *add_union_sym(char *name) {
    Symbol *sym = new_sym(name, P_UNION, NULL, S_NONE, C_UNION, 0, 0);
    add_sym(&UNION_HEAD, &UNION_TAIL, sym);

    return sym;
}

Symbol *add_member_sym(char *name, int ptype, Symbol *ctype, int stype, int n_elem) {
    Symbol *sym = new_sym(name, ptype, ctype, stype, C_MEMBER, n_elem, 0);
    if (ptype == P_STRUCT || ptype == P_UNION) {
        sym->size = ctype->size;
    }
    add_sym(&MEMBER_HEAD, &MEMBER_TAIL, sym);

    return sym;
}

Symbol *add_enum_sym(char *name, int class, int value) {
    Symbol *sym = new_sym(name, P_INT, NULL, S_NONE, class, 0, value);
    add_sym(&ENUM_HEAD, &ENUM_TAIL, sym);

    return sym;
}

Symbol *add_typedef_sym(char *name, int ptype, Symbol *ctype) {
    Symbol *sym = new_sym(name, ptype, ctype, S_NONE, C_TYPEDEF, 0, 0);
    add_sym(&TYPEDEF_HEAD, &TYPEDEF_TAIL, sym);

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

Symbol *find_global_sym(char *s) {
    return find_sym_in_list(s, GLOBAL_HEAD);
}

static Symbol *find_param_sym(char *s) {
    if (FUNC_PTR) {
        return find_sym_in_list(s, FUNC_PTR->first);
    }

    return NULL;
}

Symbol *find_local_sym(char *s) {
    Symbol *sym;

    return (sym = find_param_sym(s)) ? sym : find_sym_in_list(s, LOCAL_HEAD);
}

Symbol *find_struct_sym(char *s) {
    return find_sym_in_list(s, STRUCT_HEAD);
}

Symbol *find_union_sym(char *s) {
    return find_sym_in_list(s, UNION_HEAD);
}

Symbol *find_member_sym(char *s) {
    return find_sym_in_list(s, MEMBER_HEAD);
}

/*
 * Finds the enum type symbol.
 * Just find twice:
 * - If find one, then check if it's a enum type symbol.
 * - If not, then find the second one.
 * - There will never be a third symbol with the same name.
 *
 * Note that second search should start at the first result,
 * or we'll get the same result as the first search.
 *
 * Note:
 * Same thing below(find_enum_val_sym).
 */
Symbol *find_enum_type_sym(char *s) {
    Symbol *sym = find_sym_in_list(s, ENUM_HEAD);
    return sym == NULL
           ? sym
           : (sym->class == C_ENUMTYPE ? sym : find_sym_in_list(s, sym));
}

Symbol *find_enum_val_sym(char *s) {
    Symbol *sym = find_sym_in_list(s, ENUM_HEAD);
    return sym == NULL
           ? sym
           : (sym->class == C_ENUMVAL ? sym : find_sym_in_list(s, sym));
}

Symbol *find_typedef_sym(char *s) {
    return find_sym_in_list(s, TYPEDEF_HEAD);
}

Symbol *find_sym(char *s) {
    Symbol *sym;

    return (sym = find_local_sym(s)) ? sym : find_global_sym(s);
}
