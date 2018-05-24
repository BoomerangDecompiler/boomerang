int main(int argc, char *argv[]);
void test(int param1, int param2, int param3);

/** address: 0x000106e8 */
int main(int argc, char *argv[])
{
    test(4, 5, 6);
    printf(0x10848);
    test(6, 5, 4);
    printf(0x10860);
    test(4, 6, 5);
    printf(0x10878);
    test(6, 4, 5);
    printf(0x10890);
    return 0;
}

/** address: 0x00010688 */
void test(int param1, int param2, int param3)
{
    if (param1 < param2 || param2 < param3) {
    }
    else {
    }
    return;
}

