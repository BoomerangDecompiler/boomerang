int main(int argc, union { __size32; char *[] *; } argv);

/** address: 0x100004f0 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    int g5; 		// r5

    g5 = *(argv + 4);
    if (argc > 2) {
        strlen(g5);
        printf(0x1000092c);
    }
    else {
        strlen(g5);
    }
    printf(0x10000928);
    printf(0x10000930);
    return 0;
}

