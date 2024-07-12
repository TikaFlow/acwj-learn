typedef struct abc abc;
struct abc foo;
struct abc;

struct abc* ptr;
struct abc createAbc();

struct abc {
    int a;
    struct abc *b;
};
struct abc aa;

int main() {
    return 0;
}