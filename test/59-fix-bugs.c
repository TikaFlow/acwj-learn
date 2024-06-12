#include <stdio.h>

char *argv[] = {"unused", "-fish", "-cat", "owl"};
int argc = 4;

int main() {
    int i;

    for (i = 1; i < argc; i++) {
        printf("i is %d\n", i);
        if (*argv[i] != '-') break;
    }

    while (i < argc) {
        printf("leftover %s\n", argv[i]);
        i++;
    }

    char a = 1;
    short b = 2;
    int c = 3;
    long d = 4;
    if (a == 1 && b == 2 && c == 3 && d == 4) {
        printf("All numbers are equal\n");
    }

    printf("%d\n", b > 1 ? 1 : 0);

    return (0);
}