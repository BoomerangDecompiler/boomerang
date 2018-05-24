int main(int argc, char *argv[]);

/** address: 0x10000460 */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    printf(0x100008a0);
    printf(0x100008a0);
    printf(0x100008a0);
    g4 = argc;
    if (argc < -2) {
        g4 = -2;
    }
    if (g4 > 3) {
    }
    printf(0x100008a0);
    printf(0x100008a0);
    return 0;
}

