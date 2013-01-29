// address: 8048340
int main(int argc, int argv, int envp) {
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{25}

    eax = 0;
    do {
        eax_1 = eax;
        *(__size8*)(eax_1 + esp - 76) = 0;
        eax = eax_1 + 1;
    } while (eax_1 + 1 <= 63);
    return 0;
}

