int main(int argc, char *argv[]);

/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    __size32 g31; 		// r31

    g31 = 1;
    if (argc <= 1) {
        printf("Result is %d\n", 0);
    }
    else {
        printf("Result is %d\n", 1);
    }
    return g31;
}

