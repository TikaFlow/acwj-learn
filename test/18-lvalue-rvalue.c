#include <stdio.h>
int main() {
    int a;
    int b;
    int *c;

    a = b = 34;
    printf("%d\n", a); // 34
    printf("%d\n", b); // 34
    // a
    /*
     *
     * block comment test
     */

    a = b + 45;
    printf("%d\n", a); // 79
    // local variable should - 1
    // line comment test
    /*
     *
     * block comment test
     */

    return (0);
}