long d, f;
long *e;

int main() {
    int a, b, c;
    b = 3;
    c = 5;
    a = b + c * 10;
    printf("%d\n", a);

    d = 1234567890123;
    printf("%ld\n", d);
    e = &d;
    f = *e;
    printf("%ld\n", f);
    return (0);
}