#include <stdio.h>
int main()
{
  int i; char j;

  j= 20; printf("%d\n", j); // 20
  i= 10; printf("%d\n", i); // 10

  for (i= 1;   i <= 5; i= i + 1) { printf("%d\n", i); } // 1-5
  for (j= 253; j != 2; j= j + 1) { printf("%d\n", j); } // 253 254 255 0 1

  return 0;
}