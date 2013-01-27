__size32 stat();

// address: 0x10000440
int main(int argc, char *argv[], char *envp[]) {
    int g3; 		// r3
    int g3_1; 		// r3
    int local0; 		// m[g1 - 84]

    g3_1 = stat();
    printf("Stat returns %d; size of file is %d\n", g3_1, local0);
    return g3;
}

// address: 0x100005d0
__size32 stat() {
    __size32 g3; 		// r3

    __xstat();
    return g3;
}

