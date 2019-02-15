int main(int argc, char *argv[]);


/** address: 0x00001d54 */
int main(int argc, char *argv[])
{
    __size32 g30; 		// r30

    g30 = 1;
    if (argc <= 1) {
        g30 = 0;
    }
    printf(/* machine specific */ (int) LR + 656);
    return g30;
}

