#include <stdio.h>

int main(void) {
    int x = 0x4a;
    char c = '\042';
    char d = '\x42';
    printf("x = %d\n", x);
    printf("ascii code: c = %d, d = %d\n", c, d);
    printf("char code: c = %c, d = %c\n", c, d);

    return 0;
}