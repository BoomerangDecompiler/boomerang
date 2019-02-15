int main(int argc, char *argv[]);


/** address: 0x10000468 */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    puts("Figure 19.2");
    putchar('1');
    g4 = 0;
    if (argc <= 3) {
        g4 = argc;
    }
    printf("C is %d\n", g4 + argc);
    return /* machine specific */ (int) LR;
}

