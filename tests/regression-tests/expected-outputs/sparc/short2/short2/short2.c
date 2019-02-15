int main(int argc, char *argv[]);
__size32 test(int param1, int param2, int param3);


/** address: 0x000106b4 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    o0 = test(4, 5, 6);
    printf("Result for 4, 5, 6: %d\n", o0);
    o0 = test(6, 5, 4);
    printf("Result for 6, 5, 4: %d\n", o0);
    o0 = test(4, 6, 5);
    printf("Result for 4, 6, 5: %d\n", o0);
    o0 = test(6, 4, 5);
    printf("Result for 6, 4, 5: %d\n", o0);
    return 0;
}

/** address: 0x00010688 */
__size32 test(int param1, int param2, int param3)
{
    int g1; 		// r1
    int o0; 		// r8

    g1 = 1;
    o0 = 1;
    if (param1 >= param2) {
        o0 = 0;
    }
    if (param2 >= param3) {
        g1 = 0;
    }
    return o0 & g1;
}

