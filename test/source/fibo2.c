/* This version of fibo passes parameters and the return value via a
  dummy function fib1 (in RISC machines, there will be no code to pass
  parameters or return the value. Call graph:
   main
    |
    f1<--+
    |    |
    f2---+
Compile with gcc -O4 -fno-inline -o test/sparc/fibo2 test/source/fibo2.c
*/

#include <stdio.h>

int fib2(int x);

int fib1 (int x) {
    return fib2(x);
}

int fib2 (int x)
{
	if (x > 1)
		return (fib1(x - 1) + fib1(x - 2));
	else return (x);
}

int main (void)
{	int number, value;

	printf ("Input number: ");
	scanf ("%d", &number);
	value = fib1(number);
	printf("fibonacci(%d) = %d\n", number, value);
	return (0);
}

