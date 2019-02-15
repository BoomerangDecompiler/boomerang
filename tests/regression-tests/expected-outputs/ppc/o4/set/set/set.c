int main(int argc, char *argv[]);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    if (argc <= 1) {
        printf("Result is %d\n", 0);
        g3 = 0;
    }
    else {
        printf("Result is %d\n", 1);
        g3 = 1;
    }
    return g3;
}

