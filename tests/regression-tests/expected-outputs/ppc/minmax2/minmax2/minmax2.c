int main(int argc, char *argv[]);
void test(int param1);


/** address: 0x1000048c */
int main(int argc, char *argv[])
{
    test(-5);
    test(-2);
    test(0);
    test(argc);
    test(5);
    return 0;
}

/** address: 0x10000418 */
void test(int param1)
{
    int local0; 		// m[g1 - 24]

    local0 = param1;
    if (param1 < -2) {
        local0 = -2;
    }
    if (local0 > 3) {
        local0 = 3;
    }
    printf("MinMax result %d\n", local0);
    return;
}

