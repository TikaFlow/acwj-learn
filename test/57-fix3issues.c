#include <stdio.h>
#include <errno.h>
#include <string.h>

int list[] = {3, 5, 7, 9, 11, 13, 15};
int *lptr;

int main() {

    char foo = 'c';
    char *filename = "/root/fred";
    char *a, *b, *c;

    a = b = c = NULL;
    if (a == NULL || b == NULL || c == NULL) {
        printf("One of the three is NULL\n");
    }

    a = &foo;
    if (a == NULL || b == NULL || c == NULL) {
        printf("One of the three is NULL\n");
    }

    b = &foo;
    if (a == NULL || b == NULL || c == NULL) {
        printf("One of the three is NULL\n");
    }

    c = &foo;
    if (a == NULL || b == NULL || c == NULL) {
        printf("One of the three is NULL\n");
    } else {
        printf("All  three  are non-NULL\n");
    }

    fopen(filename, "r");
    fprintf(stdout, "Unable to open %s: %s\n", filename, strerror(errno));

    char *str = "qwertyuiop";
    printf("%c ", *str);
    str = str + 1;
    printf("%c ", *str);
    str += 1;
    printf("%c ", *str);
    str++;
    printf("%c ", *str);
    str -= 1;
    printf("%c ", *str);
    str--;
    printf("%c\n", *str);

    lptr = list;
    printf("%d ", *lptr);
    lptr = lptr + 1;
    printf("%d ", *lptr);
    lptr += 1;
    printf("%d ", *lptr);
    lptr++;
    printf("%d ", *lptr);
    lptr -= 1;
    printf("%d ", *lptr);
    lptr--;
    printf("%d\n", *lptr);

    return (0);
}