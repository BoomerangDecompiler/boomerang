int main(int argc, char *argv[]);
void foo1();
void foo2();

union { double; __size32; } a;
int b = 7;

/** address: 0x00010754 */
int main(int argc, char *argv[])
{
    foo1();
    printf("b = %i\n", b);
    return 0;
}

/** address: 0x0001073c */
void foo1()
{
    foo2();
    return;
}

/** address: 0x000106fc */
void foo2()
{
    b = 12;
    printf("a = %f\n", a);
    return;
}

