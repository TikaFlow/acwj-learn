#include <stdio.h>

static int a = 0;
int xx = 1;
static int b = 0;

static void aa() { a = 1; }

int bb() { return 1; }

static int counter = 0;

static int fred(void) { return (counter++); }

static int c = 0;

int main() {
    int i;
    for (i = 0; i < 5; i++) {
        printf("%d\n", fred());
    }
    return 0;
}

static int d = 0;