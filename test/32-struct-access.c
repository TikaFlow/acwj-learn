#include <stdio.h>
struct fred {
    int x;
    char y;
    long z;
};

struct fred var;
struct fred *varptr;

int main() {
    long result;

    var.x = 12345;
    printf("%d\n", var.x);
    var.y = 65; // ASCII code for 'A'
    printf("%c\n", var.y);
    var.z = 0x1234567890; // decimal value is 78187493520
    printf("%ld\n", var.z);

    result = var.x + var.y + var.z;
    printf("%ld\n", result); // 78187505930

    varptr = &var;
    result = varptr->z - varptr->y - varptr->x;
    printf("%ld\n", result); // 78187481110
    return (0);
}