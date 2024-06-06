#include <stdio.h>

struct foo {
    int x;
    int y;
} fred, *mary;

int main() {
    int a, b, *c;
    a = 5;
    c = &a;
    b = *c + 1;
    printf("b = %d\n", b);

    mary = &fred;
    mary->x = 10;
    mary->y = 20;

    printf("so in fred, x = %d and y = %d\n", fred.x, fred.y);

    return 0;
}