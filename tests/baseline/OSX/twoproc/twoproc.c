__size32 proc1(__size32 param2, __size32 param2);

// address: 0x1d80
int main(int argc, char *argv[], char *envp[]) {
    int g3; 		// r3

    g3 = proc1(11, 4);
    printf(/* machine specific */ (int) LR + 324);
    return g3;
}

// address: 0x1d50
__size32 proc1(__size32 param2, __size32 param2) {
    return param2 - param2;
}

