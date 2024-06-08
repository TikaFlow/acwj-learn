#include <stdio.h>

struct foo {
    int a;
    long b;
    char c;
};

typedef struct foo bar;
bar baz;
char y = 128;

int main(void) {
    short x = 32768;
    printf("short x is %d\n", x);
    printf("char y is %d\n", y);
    printf("size of char: %zd\n", sizeof(char));
    printf("size of short: %zd\n", sizeof(short));
    printf("size of int: %zd\n", sizeof(int));
    printf("size of foo: %zd\n", sizeof(struct foo));
    printf("size of alias of foo: %zd\n", sizeof(bar));
    printf("size of baz(type is bar): %zd\n", sizeof(baz));

    return 0;
}