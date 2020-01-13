int main(int argc, char *argv[]);
void foo1();
void foo2();

union { double; __size32; } a;
int b = 7;

/** address: 0x08048363 */
int main(int argc, char *argv[])
{
    foo1();
    printf("b = %i\n", b);
    return 0;
}

/** address: 0x08048356 */
void foo1()
{
    foo2();
    return;
}

/** address: 0x08048328 */
void foo2()
{
    b = 12;
    printf("a = %f\n", a);
    return;
}

