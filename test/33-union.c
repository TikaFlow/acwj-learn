#include <stdio.h>
union foo_union {
    char w;
    int x;
    int y;
    long z;
};

union foo_union var;
union foo_union *varptr;

int main() {
    var.x = 0b01000001; // ASCII for 'A'
    printf("%c\n", var.x);
    var.x = 0102; // decimal is 66
    printf("%d\n", var.x);
    printf("%d\n", var.y);
    printf("The next two depend on the endian of the platform\n");
    printf("%d\n", var.w);
    printf("%ld\n", var.z);

    varptr = &var;
    varptr->x = 97;
    printf("w is %c\n", varptr->w);
    printf("x is %d\n", varptr->x);
    printf("y is %d\n", varptr->y);
    printf("z is %ld\n", varptr->z);

    return (0);
}