//
// Created by tika on 24-5-21.
//

#include "data.h"
#include "decl.h"

void match(int tokentype, char *what) {
    if (TOKEN.token == tokentype) {
        scan(&TOKEN);
        return;
    }
    printf("'%s' expected on line %d\n", what, LINE);
    exit(1);
}

void semi() {
    match(T_SEMI, ";");
}
