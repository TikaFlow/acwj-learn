int c;
int d;
int *e;
int f;

int main() {
    c = 12;
    d = 18;
    printd(c); // 12
    e = &c + 1;
    f = *e;
    printd(f); // 18
    return (0);
}