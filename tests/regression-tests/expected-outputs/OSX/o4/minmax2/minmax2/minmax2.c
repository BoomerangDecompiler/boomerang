int main(int argc, char *argv[]);


/** address: 0x00001ccc */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    printf(/* machine specific */ (int) LR + 788);
    printf(/* machine specific */ (int) LR + 788);
    printf(/* machine specific */ (int) LR + 788);
    g4 = argc;
    if (argc < -2) {
        g4 = -2;
    }
    if (g4 <= 3) {
    }
    printf(/* machine specific */ (int) LR + 788);
    printf(/* machine specific */ (int) LR + 788);
    return 0;
}

