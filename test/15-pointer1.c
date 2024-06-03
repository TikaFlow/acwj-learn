#include <stdio.h>
int main() {
  char  a; char *b; char  c;
  int   d; int  *e; int   f;
  long  g; long *h; long  i;

  a= 255; printf("%d\n", a);
  b= &a; c= *b; printf("%d\n", c);

  d= 123457890; printf("%d\n", d);
  e= &d; f= *e; printf("%d\n", f);

  g= 1234567890123; printf("%ld\n", g);
  h= &g; i= *h;     printf("%ld\n", i);

  return(0);
}