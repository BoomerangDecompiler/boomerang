// address: 106a8
int main(int argc, char *argv[], char *envp[]) {
    struct stat local0; 		// m[o6 - 152]
    int local1; 		// m[o6 - 104]
    int o0; 		// r8
    int o0_1; 		// r8

    stat("test/source/stattest.c", &local0);
    printf("Stat returns %d; size of file is %d\n", o0_1, local1);
    return o0;
}

