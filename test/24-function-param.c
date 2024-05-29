int param8(int a, int b, int c, int d, int e, int f, int g, int h) {
    printd(a); printd(b); printd(c); printd(d);
    printd(e); printd(f); printd(g); printd(h);
    return(0);
}

int param5(int a, int b, int c, int d, int e) {
    printd(a); printd(b); printd(c); printd(d); printd(e);
    return(0);
}

int param2(int a, int b) {
    int c; int d; int e;
    c= 3; d= 4; e= 5;
    printd(a); printd(b); printd(c); printd(d); printd(e);
    return(0);
}

int param0() {
    int a; int b; int c; int d; int e;
    a= 1; b= 2; c= 3; d= 4; e= 5;
    printd(a); printd(b); printd(c); printd(d); printd(e);
    return(0);
}

/*
 * for now, the main function should compile with gcc
 *
extern int param8(int a, int b, int c, int d, int e, int f, int g, int h);
extern int param5(int a, int b, int c, int d, int e);
extern int param2(int a, int b);
extern int param0();

int main() {
    param8(1,2,3,4,5,6,7,8);
    param5(1,2,3,4,5);
    param2(1,2);
    param0();
    return(0);
}
*/
