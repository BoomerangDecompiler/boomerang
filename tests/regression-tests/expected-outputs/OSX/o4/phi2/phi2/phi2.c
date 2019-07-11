int main(int argc, char *argv[]);


/** address: 0x00001c60 */
int main(int argc, char *argv[])
{
    int g28; 		// r28
    int g29; 		// r29
    int g3; 		// r3
    char *g30; 		// r30
    int g3_2; 		// r3{6}
    int g3_5; 		// r3{8}

    g30 = *(argv + 4);
    if (argc <= 2) {
        g3 = strlen(g30);
        g29 = g3;
    }
    else {
        g3_2 = strlen(g30);
        g29 = g3_2;
        g3_5 = strlen(g30);
        g28 = g3_5;
        printf("%d", g3_2 + g3_5);
    }
    printf("%d, %d", g29, g28);
    printf("%d\n", g29);
    return 0;
}

