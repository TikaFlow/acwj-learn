int param8(int a, int b, int c, int d, int e, int f, int g, int h) {
  printd(a); printd(b); printd(c); printd(d);
  printd(e); printd(f); printd(g); printd(h);
  return(0);
}

int fred(int a, int b, int c) {
  return(a+b+c);
}

int main() {
  int x;
  param8(1, 2, 3, 5, 8, 13, 21, 34);
  x= fred(2, 3, 4); printd(x);
  return(0);
}