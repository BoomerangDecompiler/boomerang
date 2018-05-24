int main(int argc, char *argv[]);
void foo1();
void foo2();

/** address: 0x00010750 */
int main(int argc, char *argv[])
{
    foo1();
    printf(0x10838);
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
    *(int*)0x209b4 = 12;
    printf(0x10830);
    return;
}

