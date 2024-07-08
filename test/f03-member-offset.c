#include <stdio.h>

// fix: cannot get member after nested struct/union
// fix: wrong offset after a nested struct/union
struct abc {
    int a;
    union {
        int ba;
        int bb;
    };
    int c;
    struct {
        int da;
        int db;
    } ddd;
};

int main() {
    struct abc foo;
    foo.bb = -257;
    foo.c = 257;
    foo.ddd.da = -2517;

    printf("%d\n", foo.ba);
    printf("%d\n", foo.ba);
    printf("%d\n", foo.ba);
    printf("%d\n", foo.c);
    printf("%d\n", foo.ddd.db);

    return 0;
}