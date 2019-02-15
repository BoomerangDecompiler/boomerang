int main(int argc, char *argv[]);
__size32 test(int param1);


/** address: 0x00001d44 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g30; 		// r30

    test(-5);
    test(-2);
    g30 = test(0);
    g3 = *(g30 + 104);
    test(g3);
    test(5);
    return 0;
}

/** address: 0x00001cd0 */
__size32 test(int param1)
{
    __size32 g1; 		// r1
    int local0; 		// m[g1 + 24]

    local0 = param1;
    if (param1 < -2) {
        local0 = -2;
    }
    if (local0 <= 3) {
    }
    printf(/* machine specific */ (int) LR + 772);
    return (g1 - 80);
}

