#include <stdio.h>

int fib (int x)
{
    int n, fibn, fibn_1, save;
	if (x <= 1)
		return x;
    n = 2;
    fibn = 1;       /* fib(2) = 1 */
    fibn_1 = 1;     /* fib(1) = 1 */
	while (n < x) {
        save = fibn;
        fibn = fibn + fibn_1;   /* fib(n+1) = fib(n) + fib(n-1) */
        fibn_1 = save;          /* fib(n-1) = old fib(n) */
        n++;
    }
    return fibn;
}

int main (void)
{	int number, value;

	printf ("Input number: ");
	scanf ("%d", &number);
	value = fib(number);
	printf("fibonacci(%d) = %d\n", number, value);
	return (0);
}

