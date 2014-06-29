__size32 foo1();
__size32 foo2();

// address: 0x1d4c
int main(int argc, char *argv[], char *envp[]) {
    __size32 g31; 		// r31

    g31 = foo1();
    printf(g31 + 660);
    return 0;
}

// address: 0x1d20
__size32 foo1() {
    __size32 g31; 		// r31
    __size32 g31_1; 		// r31{72}

    g31_1 = foo2();
    return g31; /* WARNING: Also returning: g31 := g31_1 */
}

// address: 0x1cc4
__size32 foo2() {
    __size32 g31; 		// r31

    *(__size32*)(/* machine specific */ (int) LR + 844) = 12;
    printf(/* machine specific */ (int) LR + 784);
    return g31; /* WARNING: Also returning: g31 := /* machine specific */ (int) LR */
}

