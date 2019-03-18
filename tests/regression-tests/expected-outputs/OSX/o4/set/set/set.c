int main(int argc, char *argv[]);


/** address: 0x00001d54 */
int main(int argc, char *argv[])
{
    int g30; 		// r30

    g30 = 1;
    if (argc <= 1) {
        g30 = 0;
    }
    printf("Result is %d\n", g30);
    return g30;
}

