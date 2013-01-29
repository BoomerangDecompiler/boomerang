// address: 8048334
int main(int argc, int argv, int envp) {
    union { __size8 * x1; int x2; } eax; 		// r24
    void *esp; 		// r28
    int local0; 		// m[esp - 80]

    local0 = 0;
    while (local0 <= 63) {
        eax = esp + local0 - 76;
        *(__size8*)eax = 0;
        local0++;
    }
    return 0;
}

