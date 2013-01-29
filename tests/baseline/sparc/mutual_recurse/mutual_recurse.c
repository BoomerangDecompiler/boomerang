__size32 c1(int param1);
void c2(int param1);

// address: 1062c
int main(int argc, char *argv[], char *envp[]) {
    int o0; 		// r8

    o0 = c1(argc);
    printf("Result is %d\n", o0);
    return 0;
}

// address: 10650
__size32 c1(int param1) {
    __size32 i0_1; 		// r24{28}
    int i0_2; 		// r24{37}
    __size32 i1_1; 		// r25{32}
    int i1_2; 		// r25{37}
    int i2; 		// r26
    int o0; 		// r8
    __size32 o0_1; 		// r8{74}

    i0_1 = 0;
    i1_1 = 0;
    if (param1 > 0) {
        do {
            i1_1++;
            c2(o0_1);
            i0_1 = i0_2 + o0;
        } while (i1_2 < i2);
    }
    return i0_1;
}

// address: 1068c
void c2(int param1) {
    if ((param1 & 0x1) != 0) {
        c1(param1);
    }
    return;
}

