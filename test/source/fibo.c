#include <stdio.h>

int fib (int x)
{
	if (x > 1)
		return (fib(x - 1) + fib(x - 2));
	else return (x);
}

int main (void)
{	int number, value;

	printf ("Input number: ");
	scanf ("%d", &number);
	value = fib(number);
	printf("fibonacci(%d) = %d\n", number, value);
	return (0);
}

