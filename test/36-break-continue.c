#include <stdio.h>

int main() {
    int x;
    x = 0;
    while (x < 100) {
        if (x == 5) {
            x = x + 2;
            continue;
        }
        if (x <= 14) {
            printf("loop go on...\n");
        } else {
            break;
        }

        printf("%d\n", x);
        x = x + 1;
    }

    printf("Done\n");
    return (0);
}