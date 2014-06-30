// address: 0x8048328
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    int local1; 		// m[esp - 12]

    printf("Figure 19.2\n");
    local1 = 0;
    printf("1");
    if (argc <= 3) {
        local1 = argc;
    }
    eax = argc + local1;
    printf("C is %d\n", eax);
    return eax;
}

