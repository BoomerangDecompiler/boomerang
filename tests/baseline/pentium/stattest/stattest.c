// address: 0x804835c
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    struct stat local0; 		// m[esp - 108]
    int local1; 		// m[esp - 64]

    stat(3, "test/source/stattest.c", &local0);
    printf("Stat returns %d; size of file is %d\n", eax, local1);
    return eax;
}

