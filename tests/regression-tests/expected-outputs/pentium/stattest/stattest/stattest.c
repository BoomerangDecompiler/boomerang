int main(int argc, char *argv[]);


/** address: 0x0804835c */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    struct stat local0; 		// m[esp - 108]
    int local1; 		// m[esp - 64]

    eax = stat(3, "test/source/stattest.c", &local0);
    printf("Stat returns %d; size of file is %d\n", eax, local1);
    return eax;
}

