int main(int argc, char *argv[]);


/** address: 0x00001ccc */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    printf("MinMax result %d\n", -2);
    printf("MinMax result %d\n", -2);
    printf("MinMax result %d\n", 0);
    g4 = argc;
    if (argc < -2) {
        g4 = -2;
    }
    if (g4 > 3) {
        g4 = 3;
    }
    printf("MinMax result %d\n", g4);
    printf("MinMax result %d\n", 3);
    return 0;
}

