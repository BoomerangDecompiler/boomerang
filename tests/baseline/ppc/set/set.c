// address: 0x10000418
int main(int argc, char *argv[], char *envp[]) {
    unsigned int g0; 		// r0
    unsigned char local0; 		// m[g1 - 19]
    unsigned char local1; 		// m[g1 - 20]

    local0 = 1;
    if (argc <= 1) {
        local0 = 0;
    }
    local1 = (char) (local0);
    g0 = (local1);
    printf("Result is %d\n", ROTL(g0) & 0xff);
    g0 = (local1);
    return ROTL(g0) & 0xff;
}

