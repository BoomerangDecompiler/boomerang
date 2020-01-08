int main(int argc, char *argv[]);
void foo1();
void foo2();

int a;
int b;

/** address: 0x00001d50 */
int main(int argc, char *argv[])
{
    foo1();
    printf("b = %i\n", b);
    return 0;
}

/** address: 0x00001d24 */
void foo1()
{
    foo2();
    return;
}

/** address: 0x00001ccc */
void foo2()
{
    b = 12;
    printf("a = %i\n", a);
    return;
}

