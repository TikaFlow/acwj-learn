#include <stdio.h>

int main() {
    int x;
    int y;
    y = 0;

    for (x = 0; x < 5; x++) {
        switch (x) {
            case 1: {
                y = 5;
                break;
            }
            case 2: {
                y = 7;
                break;
            }
            case 3: {
                y = 9;
                // fall through
            }
            default: {
                y = 100;
            }
        }
        printf("x = %d, y = %d\n", x, y);
    }
    return (0);
}