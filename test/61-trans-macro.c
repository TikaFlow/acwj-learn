#include <stdio.h>

#define INC_DIR "/tmp/include"
#define test1 100
#define test2 123
#define test3 (-333)

int main() {
    printf("macro INC_DIR = %s\n", INC_DIR);
    printf("macro test1 = %d\n", test1);
    printf("macro test2 = %d\n", test2);
    printf("macro test3 = %d\n", test3);

    return 0;
}