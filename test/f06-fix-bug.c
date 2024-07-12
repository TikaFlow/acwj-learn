struct foo;
// struct foo f2[3]; // illegal
// void fred(struct foo a) { // illegal
//     struct foo f3[3]; // illegal
//     f3[1].b = 10;
// }
struct foo {
    int a;
    int b;
};
struct foo f2[3];
struct bar {
    struct foo f;
    int i;
};

void fred() {
    struct foo f3[3];
    f3[1].b = 10;
}

struct bar bbb;

int main() {
    return 0;
}