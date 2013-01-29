// address: 8048370
int main(int argc, char *argv[], char *envp[]) {
    unsigned char cl; 		// r9
    unsigned char dl; 		// r10

    printf("Hello, set\n");
    dl =  (argc < 3) ? 1 : 0;
    printf("argc <u 3: %d\n", 0 >> 8 & 0xffffff | (dl));
    cl =  (argc - 4 >= 0) ? 1 : 0;
    printf("(argc - 4) >= 0: %d\n", 0 >> 8 & 0xffffff | (cl));
    return 0;
}

