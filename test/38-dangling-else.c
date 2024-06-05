#include <stdio.h>

int main() {
    int a, i;
    a = 20;

    for (i = 1; i <= a; i++) {
        if (i <= 10) i = i + 1;
        else i = i + 2;
        printf("i = %d\n", i);
    }

    printf("finally, i = %d\n", i);

    switch (a) {
        case 20:
        case 21:
            printf("for now, a = %d\n", a);
    }

    for (; a < i;)
        printf("a = %d\n", a++);

    return 0;
}