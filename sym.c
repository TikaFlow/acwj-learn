//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

static void add_sym(Symbol **head, Symbol **tail, Symbol *sym);

static void dump_table(Symbol *head, char *name, int indent);

void reset_global_syms() {
    GLOBAL_HEAD = GLOBAL_TAIL = NULL;
    STRUCT_HEAD = STRUCT_TAIL = NULL;
    UNION_HEAD = UNION_TAIL = NULL;
    ENUM_HEAD = ENUM_TAIL = NULL;
    TYPEDEF_HEAD = TYPEDEF_TAIL = NULL;
}

void reset_local_syms() {
    LOCAL_HEAD = LOCAL_TAIL = NULL;
    PARAM_HEAD = PARAM_TAIL = NULL;
    FUNC_PTR = NULL;
}

void reset_static_syms() {
    Symbol *cur, *prev = NULL;

    for (cur = GLOBAL_HEAD; cur; cur = cur->next) {
        if (cur->class == C_STATIC) {
            printf("Releasing static %s %s\n", cur->stype == S_FUNCTION ? "function" : "variable", cur->name);
            if (cur == GLOBAL_HEAD) {
                GLOBAL_HEAD = cur->next;
            } else {
                prev->next = cur->next;
            }

            if (cur == GLOBAL_TAIL) {
                if (!prev) {
                    GLOBAL_TAIL = prev;
                }
            }

        }
        prev = cur;
    }

    printf("\n");
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

    if (name) {
        sym->name = strdup(name);
    } else {
        sym->name = name;
    }
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
    Symbol *sym = new_sym(name, P_STRUCT, NULL, S_TYPE, C_STRUCT, 0, 0);

    add_sym(&STRUCT_HEAD, &STRUCT_TAIL, sym);

    return sym;
}

Symbol *add_union_sym(char *name) {
    Symbol *sym = new_sym(name, P_UNION, NULL, S_TYPE, C_UNION, 0, 0);
    add_sym(&UNION_HEAD, &UNION_TAIL, sym);

    return sym;
}

Symbol *add_member_sym(char *name, Symbol **head, Symbol **tail, int ptype, Symbol *ctype, int stype, int n_elem) {
    Symbol *sym = new_sym(name, ptype, ctype, stype, C_MEMBER, n_elem, 0);
    if (ptype == P_STRUCT || ptype == P_UNION) {
        sym->size = ctype->size;
    }
    add_sym(head, tail, sym);

    return sym;
}

Symbol *add_enum_sym(char *name, int class, int value) {
    Symbol *sym = new_sym(name, P_INT, NULL, S_VARIABLE, class, 0, value);
    add_sym(&ENUM_HEAD, &ENUM_TAIL, sym);

    return sym;
}

Symbol *add_typedef_sym(char *name, int ptype, Symbol *ctype) {
    Symbol *sym = new_sym(name, ptype, ctype, S_TYPE, C_TYPEDEF, 0, 0);
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

Symbol *find_member_sym(char *s, Symbol *head) {
    return find_sym_in_list(s, head);
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

static void dump_sym(Symbol *sym, int indent) {
    int i;
    char *name = NULL;

    for (i = 0; i < indent; i++) {
        printf(" ");
    }

    // print symbol type and name
    switch (sym->ptype & (~0xf)) {
        case P_VOID:
            printf("void ");
            break;
        case P_CHAR:
            printf("char ");
            break;
        case P_SHORT:
            printf("short ");
            break;
        case P_INT:
            printf("int ");
            break;
        case P_LONG:
            printf("long ");
            break;
        case P_STRUCT:
            name = NULL;
            name = sym->ctype ? sym->ctype->name : sym->name;
        case P_UNION:
            if (sym->ptype == P_STRUCT) {
                printf("struct %s ", name ? name : "[anon]");
            } else {
                printf("union %s ", name ? name : "[anon]");
            }
            break;
        default:
            printf("unknown type ");
    }
    for (i = 0; i < (sym->ptype & 0xf); i++) {
        printf("*");
    }
    printf("%s", sym->name ? sym->name : "[anon]");

    // if function, add (); if array, add []
    switch (sym->stype) {
        case S_TYPE:
        case S_VARIABLE:
            break;
        case S_FUNCTION:
            printf("()");
            break;
        case S_ARRAY:
            printf("[]");
            break;
        default:
            printf(" unknown stype");
    }

    // print symbol class
    switch (sym->class) {
        case C_GLOBAL:
            printf(": global");
            break;
        case C_LOCAL:
            printf(": local");
            break;
        case C_PARAM:
            printf(": param");
            break;
        case C_EXTERN:
            printf(": extern");
            break;
        case C_STATIC:
            printf(": static");
            break;
        case C_STRUCT:
            printf(": struct, size: %d\n", sym->size);
            break;
        case C_UNION:
            printf(": union, size: %d\n", sym->size);
            break;
        case C_MEMBER:
            printf(": member");
            break;
        case C_ENUMTYPE:
            printf(": enumtype");
            break;
        case C_ENUMVAL:
            printf(": enumval");
            break;
        case C_TYPEDEF:
            printf(": typedef\n");
            break;
        default:
            printf(": unknown class");
    }

    // print symbol value
    switch (sym->stype) {
        case S_VARIABLE:
            switch (sym->class) {
                case C_ENUMVAL:
                    printf(", value %d\n", sym->posn);
                    break;
                case C_MEMBER:
                    printf(", offset %d", sym->posn);
                default:
                    printf(", size %d\n", sym->size);
            }
            break;
        case S_FUNCTION:
            printf(", %d params\n", sym->n_elem);
            dump_table(sym->first, NULL, 4);
            break;
        case S_ARRAY:
            printf(", %d elements, size %d\n", sym->n_elem, sym->size);
            break;
    }

    switch (sym->ptype & (~0xf)) {
        case P_STRUCT:
        case P_UNION:
            dump_table(sym->first, NULL, 4);
    }
}

static void dump_table(Symbol *head, char *name, int indent) {
    Symbol *sym;

    if (head && name) {
        printf("%s\n-----------------------\n", name);
    }

    for (sym = head; sym; sym = sym->next) {
        dump_sym(sym, indent);
    }

    if (head && name) {
        printf("\n");
    }
}

void dump_sym_tables() {
    dump_table(GLOBAL_HEAD, "Global", 0);
    dump_table(ENUM_HEAD, "Enums", 0);
    dump_table(STRUCT_HEAD, "Structs", 0);
    dump_table(UNION_HEAD, "Unions", 0);
    dump_table(TYPEDEF_HEAD, "Typedefs", 0);
}
