char a, b, c;
int  d, e, f;
long g, h, i;

char *str;
int   x;

int main() {
    b= 5; c= 7; a= b + c++; printd(a); printd(c);
    e= 5; f= 7; d= e + f++; printd(d); printd(f);
    h= 5; i= 7; g= h + i++; printd(g); printd(i);
    printc('\n');
    a= b-- + c; printd(a); printd(b);
    d= e-- + f; printd(d); printd(e);
    g= h-- + i; printd(g); printd(h);
    printc('\n');
    a= ++b + c; printd(a); printd(b);
    d= ++e + f; printd(d); printd(e);
    g= ++h + i; printd(g); printd(h);
    printc('\n');
    a= b * --c; printd(a); printd(c);
    d= e * --f; printd(d); printd(f);
    g= h * --i; printd(g); printd(i);
    printc('\n');

    x= -66 + +23; printd(x);
    printd(-10 * -10);
    x= 1; x= ~x; printd(x);
    x= 2 > 5; printd(x);
    x= !x; printd(x);
    x= !x; printd(x);
    x= 13; if (x) { printd(13); }
    x= 0; if (!x) { printd(14); }
    for (str= "Hello world\n"; *str; str++) {
        printc(*str);
    }
    printc('\n');

    d= 42; e= 19;
    printd(d & e);
    printd(d | e);
    printd(d ^ e);
    printd(1 << 3);
    printd(63 >> 3);
    printc('\n');

    prints("Everything works! It's great!\n");

    return(0);
}
