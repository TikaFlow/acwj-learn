#include <stdio.h>

int main() {
    int x = 3, y = x + 2;
    int z = y + 2 * x;
    char *str = "Hello world";
    printf("str = %s, x = %d, y = %d, z = %d\n", str, x, y, z);
    return (0);
}