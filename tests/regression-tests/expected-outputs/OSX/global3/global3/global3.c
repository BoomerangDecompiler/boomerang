int main(int argc, char *argv[]);
void foo1();
void foo2();

long long a;
int b;

/** address: 0x00001d4c */
int main(int argc, char *argv[])
{
    foo1();
    printf("b = %i\n", b);
    return 0;
}

/** address: 0x00001d20 */
void foo1()
{
    foo2();
    return;
}

/** address: 0x00001cc4 */
void foo2()
{
    b = 12;
    printf("a = %lld\n", a);
    return;
}

