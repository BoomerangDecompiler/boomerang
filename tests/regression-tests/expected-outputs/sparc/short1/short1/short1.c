int main(int argc, char *argv[]);
__size32 test(int param1, int param2, int param3);


/** address: 0x000106e8 */
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
    __size32 local0; 		// m[o6 - 20]

    if (param1 < param2 || param2 < param3) {
        local0 = 1;
    }
    else {
        local0 = 0;
    }
    return local0;
}

