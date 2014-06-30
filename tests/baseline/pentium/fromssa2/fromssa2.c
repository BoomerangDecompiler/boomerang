// address: 0x8048328
int main(int argc, char *argv[], char *envp[]) {
    __size32 ebx; 		// r27
    __size32 ebx_1; 		// r27{55}

    ebx = 0;
    do {
        ebx_1 = ebx;
        ebx = ebx_1 + 1;
        printf("%d ", ebx_1 + 1);
    } while (ebx_1 + 1 <= 9);
    printf("a is %d, x is %d\n", 10, 10);
    return 0;
}

