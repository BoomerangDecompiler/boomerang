int a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

// address: 0x1069c
int main(int argc, char *argv[], char *envp[]) {
    char * *local0; 		// m[o6 - 20]
    int local1; 		// m[o6 - 24]
    int o0; 		// r8

    local0 = 0;
    local1 = 0;
    while (local1 <= 9) {
        o0 = a[local1];
        local0 += o0;
        local1++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

