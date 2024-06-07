#include <stdio.h>

char *a = NULL;
char *b = (char *) 0;
char *c = (void *) 0;
struct foo {
    int p;
};
typedef struct foo FOO;

int main() {
    char d = (char) 65535;
    printf("d = %d\n", d);

    int e = (int) 0x1fffffff0;
    printf("e = %d\n", e);

    char *f = "abc";
    printf("str f's value = %lx\n", (long) f);

    return 0;
}