#include <stdio.h>
int c;
int d;
int *e;
int f;

int main() {
    c = 12;
    d = 18;
    printf("%d\n", c); // 12
    e = &c + 1;
    f = *e;
    printf("%d\n", f); // 18
    return (0);
}