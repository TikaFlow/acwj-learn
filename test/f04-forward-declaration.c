typedef struct abc abc;
struct abc;

// struct abc* ptr; // legal but not support now
// struct abc createAbc(); // legal but not support now

struct abc {
    int a;
    struct abc *b;
};
struct abc aa;

int main() {
    return 0;
}