// address: 8048340
int main(int argc, char *argv[], char *envp[]) {
    unsigned int eax; 		// r24
    unsigned int eax_1; 		// r24{27}

    eax = 0;
    do {
        eax_1 = eax;
        *(__size32*)(esp + eax_1 * 4 - 268) = 0;
        eax = eax_1 + 1;
    } while (eax_1 + 1 <= 63);
    return 0;
}

