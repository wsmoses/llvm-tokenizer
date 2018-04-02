//#include <stdio.h>

/*
__attribute__((const, noinline)) double norm(double*)

void normalize(unsigned int n, double* __restrict A, double* __restrict B) {
  n = 10000;
  for(int i=0; i<=n; i++) {
    A[i] = B[i] / norm(B);
  }
}
*/

int fib(int x) {
  if(x < 2) return 1;
  return fib(x-1) + fib(x-2);
}
int main() {
  printf("fib(25)=%d\n", fib(40));
}

