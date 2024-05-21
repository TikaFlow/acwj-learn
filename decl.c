//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void declarevar() {
    match(T_INT, "int");
    ident();
    addglob(TEXT);
    genglobsym(TEXT);
    semi();
}
