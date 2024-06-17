// Our first special program. It cats itself!!!
/*
 * - no pre-processor, so put the prototypes manually
 * - can't accept char* argv[], so hard-code the file name
 *
 * - Not elegant, but progressive.
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

char *buf;

int main() {
    int zin;
    int cnt;

    buf = malloc(60);
    zin = open("test/26-prototypes.c", 0);
    if (zin == -1) {
        return (1);
    }
    while ((cnt = read(zin, buf, 60)) > 0) {
        // 1 is stdout
        write(1, buf, cnt);
    }
    close(zin);
    return (0);
}
