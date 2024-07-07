#include <stdio.h>

struct abc {
    int a;
    union {
        int ba;
        int bb;
        int bc;
        struct {
            char bda;
            long bdb;
            int bdc;
            union {
                int bdda;
                int bddb;
                int bddc;
            };
        };
    };
    int c;
};
struct abc foo;

int main() {
    foo.bb = 257;
    foo.bdda = 257;

    printf("%d\n", foo.bda);
    printf("%d\n", foo.bddb);

    return 0;
}