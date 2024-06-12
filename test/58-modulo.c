#include <stdio.h>

int a;

int main() {
    printf("%d\n", 24 % 9); // 6
    printf("%d\n", 31 % 11); // 9

    a = 24;
    a %= 9;
    printf("%d\n", a);

    int b;
    b = 31;
    b %= 11;
    printf("%d\n", b);
    return (0);
}