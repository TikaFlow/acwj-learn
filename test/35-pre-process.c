#include <stdio.h>

typedef int FOO;

int main() {
    FOO x;
    x = 56;
    printf("x = %d\n", x);
    size_t y;
    y = x + 78;
    printf("y = %ld\n", y);
    return (0);
}