//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void declarevar() {
    match(T_INT, "int");
    match(T_IDENT, "identifier");
    addglob(TEXT);
    genglobsym(TEXT);
    match(T_SEMI, ";");
}
