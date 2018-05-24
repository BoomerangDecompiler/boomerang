int main(int argc, char *argv[]);
void test(int param1, int param2, int param3);

/** address: 0x000106b4 */
int main(int argc, char *argv[])
{
    test(4, 5, 6);
    printf(0x107d8);
    test(6, 5, 4);
    printf(0x107f0);
    test(4, 6, 5);
    printf(0x10808);
    test(6, 4, 5);
    printf(0x10820);
    return 0;
}

/** address: 0x00010688 */
void test(int param1, int param2, int param3)
{
    if (param1 >= param2) {
    }
    if (param2 >= param3) {
    }
    return;
}

