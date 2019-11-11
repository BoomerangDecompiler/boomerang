int main(int argc, char *argv[]);


/** address: 0x08048328 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int local1; 		// m[esp - 12]

    printf("Figure 19.2\n");
    local1 = 0;
    printf("1");
    if (argc <= 3) {
        local1 = argc;
    }
    eax = printf("C is %d\n", argc + local1);
    return eax;
}

