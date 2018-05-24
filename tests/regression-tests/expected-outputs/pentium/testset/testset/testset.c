int main(int argc, char *argv[]);

/** address: 0x08048370 */
int main(int argc, char *argv[])
{
    unsigned char cl; 		// r9
    unsigned char dl; 		// r10

    printf("Hello, set\n");
    dl =  ((unsigned int)argc < (unsigned int)3) ? 1 : 0;
    printf("argc <u 3: %d\n", 0 >> 8 & 0xffffff | (dl));
    cl =  (argc - 4 >= 0) ? 1 : 0;
    printf("(argc - 4) >= 0: %d\n", 0 >> 8 & 0xffffff | (cl));
    return 0;
}

