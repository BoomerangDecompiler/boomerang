// address: 0x80483ac
int main(int argc, char *argv[], char *envp[]) {
    __size32 edx; 		// r26
    __size32 local0; 		// m[esp - 12]
    char *local1; 		// m[esp - 16]
    int local2; 		// m[esp - 8]

    local0 = 0;
    proc1();
    proc2();
    local1 = "\x02\x04\x06\b\n";
    local2 = 0;
    while (local2 <= 4) {
        edx = (int) *local1;
        local0 += edx;
        local1++;
        local2++;
    }
    proc3();
    return 0;
}

