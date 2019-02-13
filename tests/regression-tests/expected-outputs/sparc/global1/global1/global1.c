int main(int argc, char *argv[]);
void foo1();
void foo2();

int a = 5;
int b = 7;

/** address: 0x00010750 */
int main(int argc, char *argv[])
{
    foo1();
    printf("b = %i\n", b);
    return 0;
}

/** address: 0x00010738 */
void foo1()
{
    foo2();
    return;
}

/** address: 0x000106fc */
void foo2()
{
    b = 12;
    printf("a = %i\n", a);
    return;
}

