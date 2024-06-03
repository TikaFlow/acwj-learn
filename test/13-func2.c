#include <stdio.h>
int fred() {
  return 0-20;
}

char foo(){
  return (100);
}

int main() {
  int result;
  printf("%d\n", 10);
  result = fred(15);
  printf("%d\n", result);
  result = fred(15) + 10;
  printf("%d\n", result);
  printf("%d\n", foo(15)-500);
  printf("%d\n", fred(15)-500);
  return 0;
}