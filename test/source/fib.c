int main()
{
    printf("%i\n", fib(10));
    return 0;
}

int fib(int x) {
    if (x <= 1)
        return x;
    else
        return fib(x - 1) + fib(x - 2);
}

