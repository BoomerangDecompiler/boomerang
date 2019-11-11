int main(int argc, char *argv[]);
void test(int param1);


/** address: 0x0804836f */
int main(int argc, char *argv[])
{
    test(-5);
    test(-2);
    test(0);
    test(argc);
    test(5);
    return 0;
}

/** address: 0x08048328 */
void test(int param1)
{
    int ebx; 		// r27
    int ecx; 		// r25

    ebx = -2 - param1;
    ecx = -1 - (param1 >> 31) + ((unsigned int)-2 < (unsigned int)param1);
    printf("MinMax result %d\n", (-5 - (ebx & ecx) & (-2 - (ebx & ecx) >> 31) - (-2 - (ebx & ecx) < 3)) + 3);
    return;
}

