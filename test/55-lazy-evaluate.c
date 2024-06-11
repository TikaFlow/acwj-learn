#include <stdio.h>

char *c = "str";

int main() {
    c = NULL;
    if (c && *c) {
        printf("xxx\n");
    }

    int x, y, z;
    // See if generic AND works
    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            z = x && y;
            printf("%d %d | %d\n", x, y, z);
        }
    }
    printf("\n");

    // See if generic OR works
    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            z = x || y;
            printf("%d %d | %d\n", x, y, z);
        }
    }

    return 0;
}