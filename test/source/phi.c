#include <stdio.h>
#include <string.h>

int fib (int x)
{
    int z;
    if (x > 1) {
        x = fib(x-1);
        z = fib(x-1);
        printf("%d", x+z);
        return x;
    }
    else {
        /* Force a definition of eax */
        if (x == 1) return strlen("x");
        else return (x);
    }
}

int main (void)
{   int number, value;

    printf ("Input number: ");
    scanf ("%d", &number);
    value = fib(number);
    printf("fibonacci(%d) = %d\n", number, value);
    return (0);
}

