int main(int argc, char *argv[]);

int b;

/** address: 0x00001ca4 */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    g4 = *0x2020;
    b = 12;
    printf("a = %f\n", g4);
    printf("b = %i\n", b);
    return 0;
}

