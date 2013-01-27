// address: 0x8048390
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{20}
    int esp; 		// r28
    int local0; 		// m[esp - 36]
    int local1; 		// m[esp - 40]
    int local2; 		// m[esp - 28]

    proc1();
    *(__size32*)(eax_1 + 4) = 0x8049af9;
    proc2();
    local0 = eax;
    if (eax > 10) {
        local0 = 10;
    }
    if (0 >= local0) {
        local2 = local0 * 8 - 1;
        for(;;) {
            if (local2 < 0) {
L12:
                proc3();
                proc7();
                return;
            }
            eax = esp + local2 - 124;
            if (*eax != 32) {
                goto L12;
            }
            eax = esp + local2 - 124;
            *(__size8*)eax = 0;
            local2 = local2 - 1;
        }
    }
    eax = *(eax_1 + 4);
    eax = (int) *eax;
    local1 = eax - 32;
    if (eax - 32 < 0) {
        local1 = 0;
    }
    if (local1 < 0) {
    }
    if (local1 < 0) {
    }
    proc5();
    return;
}

