
float a = 5.2f;
int b = 7;

void foo2()
{
    b = 12;
    printf("a = %f\n", a);
}

void foo1()
{
    foo2();
}

int main()
{
    foo1();
    printf("b = %i\n", b);
    return 0;
}



