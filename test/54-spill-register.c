#include <stdio.h>

int add(int x, int y) {
    return (x + y);
}

void foo() { int x = 1; }

int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;

int main() {
    int result;
    result = 3 * add(2, 3) - 5 * add(4, 6);
    printf("%d\n", result);

    int x;
    x = ((((((a + b) + c) + d) + e) + f) + g) + h;
    x = a + (b + (c + (d + (e + (f + (g + h))))));
    printf("x is %d\n", x);

    foo();
    foo();
    foo();

    return (0);
}