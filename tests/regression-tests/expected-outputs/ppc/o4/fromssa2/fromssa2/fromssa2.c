int main(int argc, char *argv[]);

/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int g9; 		// r9
    int g9_1; 		// r9{0}

    g9 = 0;
    do {
        g9_1 = g9;
        printf(0x10000828);
        g9 = g9_1 + 1;
    } while (g9_1 + 1 <= 9);
    printf(0x1000082c);
    return 0;
}

