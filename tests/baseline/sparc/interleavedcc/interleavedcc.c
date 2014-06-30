__size32 interleaved(int param1, __size32 param2, __size32 param3);

// address: 0x10acc
int main(int argc, char *argv[], char *envp[]) {
    int o0; 		// r8

    o0 = interleaved(1, 0x40000000, 0x40400000);
    printf("Call with  1, 2.0, 3.0: %d\n", o0);
    o0 = interleaved(-1, 0x40000000, 0x40400000);
    printf("Call with -1, 2.0, 3.0: %d\n", o0);
    o0 = interleaved(1, 0x40000000, 0x40000000);
    printf("Call with  1, 2.0, 2.0: %d\n", o0);
    o0 = interleaved(-1, 0x40000000, 0x40000000);
    printf("Call with -1, 2.0, 2.0: %d\n", o0);
    return 0;
}

// address: 0x10a80
__size32 interleaved(int param1, __size32 param2, __size32 param3) {
    __size32 o0; 		// r8

    if (param2 != param3) {
        o0 = 1;
        if (param1 >= 0) {
            o0 = 0;
        }
    } else {
        o0 = 2;
        if (param1 < 0) {
            o0 = 3;
        }
    }
    return o0;
}

