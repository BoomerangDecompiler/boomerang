__size32 add15(__size32 param1);
__size32 add10(__size32 param1);
__size32 add5(__size32 param1);
void printarg(int param1);

// address: 10b64
int main(int argc, char *argv[], char *envp[]) {
    __size32 o0; 		// r8

    o0 = add15(25);
    o0 = add10(o0);
    o0 = add5(o0);
    printarg(o0);
    return 0;
}

// address: 10b40
__size32 add15(__size32 param1) {
    return param1 + 15;
}

// address: 10b38
__size32 add10(__size32 param1) {
    return param1 + 10;
}

// address: 10b30
__size32 add5(__size32 param1) {
    return param1 + 5;
}

// address: 10b48
void printarg(int param1) {
    printf("Fifty five is %d\n", param1);
    return;
}

