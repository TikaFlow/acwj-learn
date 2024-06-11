#include <stdio.h>

struct foo {
    char c;
    int i;
};

int main() {
    int i;
    int ary[5];
    char z;

    z = 'H';
    for (i = 0; i < 5; i++) {
        ary[i] = i * i;
    }
    i = 14;
    for (i = 0; i < 5; i++) {
        printf("%d\n", ary[i]);
    }
    printf("%d %c\n", i, z);

    struct foo bar;
    bar.c = 'B';
    bar.i = 12;
    printf("%d %c\n", bar.i, bar.c);

    return (0);
}