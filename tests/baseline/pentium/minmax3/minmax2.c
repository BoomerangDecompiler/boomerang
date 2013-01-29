// address: 804836f
int main(int argc, char *argv[], char *envp[]) {
    test(-5);
    test(-2);
    test(0);
    test(argc);
    test(5);
    return 0;
}

// address: 8048328
void test(int param1) {
    int ebx; 		// r27
    int ebx_1; 		// r27{13}
    __size32 ecx; 		// r25
    __size32 edx; 		// r26

    ebx_1 = -2 - param1;
    ecx = -1 - (param1 >> 31) + ((unsigned int)-2 < (unsigned int)param1);
    ebx = -2 - (ebx_1 & ecx) >> 31;
    ebx = ebx - (-2 - (ebx_1 & ecx) < 3);
    edx = -5 - (ebx_1 & ecx) & ebx;
    printf("MinMax result %d\n", edx + 3);
    return;
}

