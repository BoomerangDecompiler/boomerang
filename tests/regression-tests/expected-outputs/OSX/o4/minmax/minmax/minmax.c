int main(int argc, char *argv[]);


/** address: 0x00001d30 */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    g4 = argc;
    if (argc < -2) {
        g4 = -2;
    }
    if (g4 <= 3) {
    }
    printf(/* machine specific */ (int) LR + 664);
    return 0;
}

