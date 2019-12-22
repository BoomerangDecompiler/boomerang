int main(int argc, char *argv[]);


/** address: 0x00001cf4 */
int main(int argc, char *argv[])
{
    int g29; 		// r29
    int g3; 		// r3

    g29 = 0;
    puts("Figure 19.2");
    putchar('1');
    if (argc <= 3) {
        g29 = argc;
    }
    g3 = printf("C is %d\n", g29 + argc);
    return g3;
}

