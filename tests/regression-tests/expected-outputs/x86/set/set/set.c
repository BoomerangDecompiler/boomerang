int main(int argc, char *argv[]);


/** address: 0x08048328 */
int main(int argc, char *argv[])
{
    char dl; 		// r10

    dl =  (argc > 1) ? 1 : 0;
    printf("Result is %d\n", (int) dl);
    return (int) dl;
}

