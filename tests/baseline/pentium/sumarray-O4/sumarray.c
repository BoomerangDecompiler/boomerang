// address: 0x8048328
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    int eax_1; 		// r24{41}
    __size32 edx; 		// r26

    edx = 0;
    eax = 0;
    do {
        eax_1 = eax;
        edx += a[eax_1];
        eax = eax_1 + 1;
    } while (eax_1 + 1 <= 9);
    proc1();
    return 0;
}

