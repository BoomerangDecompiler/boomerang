void test(int param1);

// address: 0x10670
int main(int argc, char *argv[], char *envp[]) {
    test(-5);
    test(-2);
    test(0);
    test(argc);
    test(5);
    return 0;
}

// address: 0x1061c
void test(int param1) {
    int g1; 		// r1

    g1 = -2 - (-2 - param1 & -1 - (param1 >> 31) + ((unsigned int)-2 < (unsigned int)param1));
    printf("MinMax result %d\n", (g1 - 3 & (g1 >> 31) - ((unsigned int)g1 < 3)) + 3);
    return;
}

