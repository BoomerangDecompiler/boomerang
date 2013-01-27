// address: 0x8048328
int main(int argc, char *argv[], char *envp[]) {
    __size32 ebx; 		// r27
    __size32 ebx_1; 		// r27{55}

    ebx = 0;
    do {
        ebx_1 = ebx;
        ebx = ebx_1 + 1;
        proc1();
    } while (ebx_1 + 1 <= 9);
    proc1();
    return 0;
}

