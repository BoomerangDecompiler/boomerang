/* Compile with
gcc -O4 -o fromssa2 -fno-unroll-loops test/source/fromssa2.c
*/
void main() {
int a, x;
a = 0;
do {
  a = a+1;
  x = a;
  printf("%d ", a);
} while (a < 10);
printf("a is %d, x is %d\n", a, x);
}

