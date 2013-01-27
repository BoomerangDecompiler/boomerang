// address: 0x8048368
int main(int argc, char *argv[], char *envp[]) {
    __size32 edx; 		// r26
    __size32 local0; 		// m[esp - 12]
    int local1; 		// m[esp - 8]

    local0 = 0;
    local1 = 0;
    while (local1 <= 4) {
        edx = (int) *(local1 + 0x80495bc);
        local0 += edx;
        local1++;
    }
    proc1();
    return 0;
}

