int main(int argc, char *argv[]);


/** address: 0x00001d30 */
int main(int argc, char *argv[])
{
    int g11; 		// r11
    int g4; 		// r4
    __size32 g4_1; 		// r4{4}
    int g5; 		// r5

    g4 = 0;
    g11 = 0x2020;
    if (SETFLAGS0(1)) {
    }
    do {
        g4_1 = g4;
        g5 = *g11;
        g11++;
        g4 = g4_1 + g5;
    } while (SETFLAGS0(1));
    printf("Sum is %d\n", g4_1 + g5);
    return 0;
}

