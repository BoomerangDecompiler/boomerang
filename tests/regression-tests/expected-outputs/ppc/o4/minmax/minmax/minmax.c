int main(int argc, char *argv[]);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    g4 = argc;
    if (argc < -2) {
        g4 = -2;
    }
    if (g4 > 3) {
        g4 = 3;
    }
    printf("MinMax adjusted number of arguments is %d\n", g4);
    return 0;
}

