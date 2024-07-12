#include <stdio.h>
char a, b, c;
int  d, e, f;
long g, h, i;

char *str;
int   x;

int main() {
    b= 5; c= 7; a= b + c++; printf("%d\n", a); printf("%d\n", c);
    e= 5; f= 7; d= e + f++; printf("%d\n", d); printf("%d\n", f);
    h= 5; i= 7; g= h + i++; printf("%ld\n", g); printf("%ld\n", i);
    printf("%c", '\n');
    a= b-- + c; printf("%d\n", a); printf("%d\n", b);
    d= e-- + f; printf("%d\n", d); printf("%d\n", e);
    g= h-- + i; printf("%ld\n", g); printf("%ld\n", h);
    printf("%c", '\n');
    a= ++b + c; printf("%d\n", a); printf("%d\n", b);
    d= ++e + f; printf("%d\n", d); printf("%d\n", e);
    g= ++h + i; printf("%ld\n", g); printf("%ld\n", h);
    printf("%c", '\n');
    a= b * --c; printf("%d\n", a); printf("%d\n", c);
    d= e * --f; printf("%d\n", d); printf("%d\n", f);
    g= h * --i; printf("%ld\n", g); printf("%ld\n", i);
    printf("%c", '\n');

    x= -66 + +23; printf("%d\n", x);
    printf("%d\n", -10 * -10);
    x= 1; x= ~x; printf("%d\n", x);
    x= 2 > 5; printf("%d\n", x);
    x= !x; printf("%d\n", x);
    x= !x; printf("%d\n", x);
    x= 13; if (x) { printf("%d\n", 13); }
    x= 0; if (!x) { printf("%d\n", 14); }
    for (str= "Hello world\n"; *str; str++) {
        printf("%c", *str);
    }
    printf("%c", '\n');

    d= 42; e= 19;
    printf("%d\n", d & e);
    printf("%d\n", d | e);
    printf("%d\n", d ^ e);
    printf("%d\n", 1 << 3);
    printf("%d\n", 63 >> 3);
    printf("%c", '\n');

    printf("%s\n", "Everything works! It's great!\n");

    return(0);
}
