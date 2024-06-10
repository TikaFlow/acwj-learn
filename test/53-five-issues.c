#include <stdio.h>

// 1. consecutive string
char *str = "abc""def";

// 2. empty statements
char foo() {
    str++;
    return *str;
}

void bar() {}

// 3. redeclare symbols
extern int fred;
int fred = 5;

// 5. return with no value
void baz() {
    printf("return with no value\n");
    return;
}

int main() {
    printf("consecutive string: %s\n", str);

    // while (foo());
    bar();

    // 4. compatible operand types
    if (*str && fred >= 5) {
        printf("compatible operand types, fred = %d\n", fred);
    }

    baz();

    return 0;
}