int main(int argc, char *argv[]);
void foo1();
void foo2();

/** address: 0x00010754 */
int main(int argc, char *argv[])
{
    foo1();
    printf(0x10840);
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
    *(int*)0x209c0 = 12;
    printf(0x10830);
    return;
}

