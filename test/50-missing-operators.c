#include <stdio.h>

int x, y, z1, z2;
enum {
    S_VARIABLE,
    S_FUNCTION,
    S_ARRAY,
};

static void aa() { x = 1; }

int bb(int abc) { return abc; }

static int counter = 0;
typedef union {
    char c;
    int i;
} un;

struct foo {
    int a;
    char b;
    long c;
} bar;

int main() {
    for (x = 0; x < 3; x++) {
        for (y = 0; y < 3; y++) {
            z1 = x || y;
            z2 = x && y;
            printf("x = %d, y = %d, x || y = %d, x && y = %d\n", x, y, z1, z2);
        }
    }

    return (0);
}