__size32 a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

// address: 0x8048328
int main(int argc, char *argv[], char *envp[]) {
    __size32 edx; 		// r26
    __size32 local0; 		// m[esp - 8]
    int local1; 		// m[esp - 12]

    local0 = 0;
    local1 = 0;
    while (local1 <= 9) {
        edx = a[local1];
        local0 += edx;
        local1++;
    }
    proc1();
    return 0;
}

