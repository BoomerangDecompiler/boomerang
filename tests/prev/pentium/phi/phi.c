int fib(int param1, __size32 param2);

// address: 0x804835c
int main(int argc, char *argv[], char *envp[]) {
    int eax; 		// r24
    __size32 edx; 		// r26

    proc1();
    proc2();
    if (eax > 1) {
        eax = fib(eax - 1, eax - 1); /* Warning: also results in edx */
        fib(eax - 1, edx);
        proc1();
L2:
    } else {
        if (eax != 1) {
            goto L2;
        }
    }
    proc1();
    return 0;
}

// address: 0x80483e0
int fib(int param1, __size32 param2) {
    int eax; 		// r24
    int ebx; 		// r27
    __size32 edx; 		// r26
    __size32 local0; 		// param2{57}

    ebx = param1;
    local0 = param2;
    if (param1 > 1) {
        proc3();
        ebx = eax;
        proc3();
        proc1();
        local0 = edx;
L1:
        param2 = local0;
        eax = ebx;
    } else {
        eax = 1;
        if (param1 != 1) {
            goto L1;
        }
    }
    return eax; /* WARNING: Also returning: edx := param2 */
}

