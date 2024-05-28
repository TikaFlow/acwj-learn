int main() {
  char  a; char *b; char  c;
  int   d; int  *e; int   f;
  long  g; long *h; long  i;

  a= 255; printd(a);
  b= &a; c= *b; printd(c);

  d= 123457890; printd(d);
  e= &d; f= *e; printd(f);

  g= 1234567890123; printd(g);
  h= &g; i= *h;     printd(i);

  return(0);
}