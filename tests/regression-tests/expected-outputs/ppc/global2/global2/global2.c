int main(int argc, char *argv[]);
void foo1();
void foo2();

/** address: 0x10000498 */
int main(int argc, char *argv[])
{
    foo1();
    printf(0x10000890);
    return 0;
}

/** address: 0x10000468 */
void foo1()
{
    foo2();
    return;
}

/** address: 0x10000418 */
void foo2()
{
    *(int*)0x100109b0 = 12;
    printf(0x10000888);
    return;
}

