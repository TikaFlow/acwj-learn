int main() {
    int a;
    int b;
    int *c;

    a = b = 34;
    printd(a); // 34
    printd(b); // 34
    // a
    /*
     *
     * block comment test
     */

    a = b + 45;
    printd(a); // 79
    // local variable should - 1
    // line comment test
    /*
     *
     * block comment test
     */
    // c = &a + 1;
    c = &a - 1;
    printd(*c); // 34
    *c = a + 10;
    printd(b); // 89

    return (0);
}