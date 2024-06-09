#include <stdio.h>

int arr[5] = {1, 2, 3, 4,};
int *ptr;
int x;

void fred(int *p) {
    printf("%d\n", p[3]);
}

int main() {
    ptr = arr;
    printf("ptr(ary) is %p\n", ptr);

    ptr = &arr;
    printf("ptr(&ary) is %p\n", ptr);

    x = arr[3];
    printf("ary[3] = %d\n", x);

    x = ptr[3];

    printf("ptr[3] = %d\n", x);
    printf("result of function receive a pointer: ");
    fred(ptr);

    ptr++;
    printf("after ptr++, ptr is %p\n", ptr);
    printf("value of ptr points: %d\n", *ptr);
    return (0);
}