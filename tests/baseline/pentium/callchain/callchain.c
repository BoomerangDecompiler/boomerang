__size32 add15(__size32 param1);
__size32 add10(__size32 param1);
__size32 add5(__size32 param1);
void printarg(int param1);

// address: 80489c4
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24

    eax = add15(25);
    eax = add10(eax);
    eax = add5(eax);
    printarg(eax);
    return 0;
}

// address: 80489a0
__size32 add15(__size32 param1) {
    return param1 + 15;
}

// address: 8048990
__size32 add10(__size32 param1) {
    return param1 + 10;
}

// address: 8048980
__size32 add5(__size32 param1) {
    return param1 + 5;
}

// address: 80489b0
void printarg(int param1) {
    printf("Fifty five is %d\n", param1);
    return;
}

