/* In Sparc, this demonstrates how an empty procedure (just a return statement)
    can pass on multiple arguments */
/* Compile with cc -xO4 -xinline= ... */

void addem(int a, int b, int c, int* res)
{
    *res = a+b+c;
}

void passem(int a, int b, int c, int* res)
{
    addem(a, b, c, res);
}

int main()
{
    int res;
    passem(5, 10, 40, &res);
    printf("Fifty five is %d\n", res);
    return 0;
}
