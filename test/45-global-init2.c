#include <stdio.h>

char *y = NULL;
int x = 10 + 6;
int fred[2 + 3];
enum abc {
    a = 10, b = a + 2, c = -1
};

int main() {
    fred[3] = x;
    printf("array fred[3] = %d\n", fred[3]);
    printf("enum a = %d, b = %d, c = %d\n", a, b, c);
    return (0);
}