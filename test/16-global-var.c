long d, f;
long *e;

int main() {
    int a, b, c;
    b = 3;
    c = 5;
    a = b + c * 10;
    printd(a);

    d = 1234567890123;
    printd(d);
    e = &d;
    f = *e;
    printd(f);
    return (0);
}