__size32 proc1(__size32 param1, __size32 param2);

// address: 0x10620
int main(int argc, char *argv[], char *envp[]) {
    __size32 o0; 		// r8

    o0 = proc1(11, 4);
    printf("%i\n", o0);
    return o0;
}

// address: 0x10618
__size32 proc1(__size32 param1, __size32 param2) {
    return param1 - param2;
}

