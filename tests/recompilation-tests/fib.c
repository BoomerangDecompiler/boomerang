#include <stdio.h>

int fib(int x)
{
    if (x <= 1)
        return x;
    else
        return fib(x - 1) + fib(x - 2);
}


int main()
{
    printf("%i\n", fib(10));
    return 0;
}
