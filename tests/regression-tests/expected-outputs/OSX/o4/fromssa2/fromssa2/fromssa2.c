int main(int argc, char *argv[]);


/** address: 0x00001d34 */
int main(int argc, char *argv[])
{
    int g9; 		// r9
    __size32 g9_1; 		// r9{2}

    g9 = 0;
    do {
        g9_1 = g9;
        printf(/* machine specific */ (int) LR + 676);
        g9 = g9_1 + 1;
    } while (g9_1 + 1 <= 9);
    printf(/* machine specific */ (int) LR + 680);
    return 0;
}

