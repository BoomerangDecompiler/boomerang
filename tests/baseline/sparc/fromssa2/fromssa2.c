// address: 10684
int main(int argc, char *argv[], char *envp[]) {
    int o0; 		// r8
    __size32 o0_1; 		// r8{73}

    o0 = 0;
    do {
        o0_1 = o0;
        printf("%d ", o0_1 + 1);
        o0 = o0_1 + 1;
    } while (o0_1 + 1 <= 9);
    printf("a is %d, x is %d\n", o0_1 + 1, o0_1 + 1);
    return 0;
}

