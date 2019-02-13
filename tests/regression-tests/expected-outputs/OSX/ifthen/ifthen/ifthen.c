int main(int argc, char *argv[]);


/** address: 0x00001cdc */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    printf(/* machine specific */ (int) LR + 748);
    printf(/* machine specific */ (int) LR + 764);
    if (argc > 3) {
    }
    g3 = printf(/* machine specific */ (int) LR + 768);
    return g3;
}

