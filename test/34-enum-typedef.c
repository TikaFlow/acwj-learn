// enum
enum fred {
    apple = 1, banana, carrot, pear = 10, peach, mango, papaya,
};
enum jane {
    aple = 1, bnana, crrot, par = 10, pech = 9, mago, paaya
} var1;

enum fred var2;

// typedef
typedef int FOO;
FOO var3;

struct bar {
    int x;
    int y
};
typedef struct bar BAR;
BAR var4;

int main() {
    var1 = carrot + pear + mango;
    printf("carrot is %d, pear is %d, mango is %d, sum is %d\n", carrot, pear, mango, var1);
    var2 = par + pech + mago;
    printf("par is %d, pech is %d, mago is %d, sum is %d\n", par, pech, mago, var2);
    printf("\n");

    var3 = 5;
    printf("%d\n", var3);
    var4.x = 7;
    var4.y = 10;
    printf("%d\n", var4.x + var4.y);
    return (0);
}