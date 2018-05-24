int main(int argc, char *argv[]);

/** address: 0x10000420 */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    printf(0x10000808);
    g3 = printf(0x10000808);
    return g3;
}

