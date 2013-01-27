// address: 0x10b0c
int main(int argc, char *argv[], char *envp[]) {
    __size32 i1; 		// r25
    __size32 o0; 		// r8
    __size32 o1; 		// r9
    __size32 o2; 		// r10
    __size32 o3; 		// r11

    o0 = 0x10a5c;
    o3 = 0x10a8c;
    o2 = 0x10abc;
    o1 = 0x10aec;
    if (o0 != 0x10a5c) {
L5:
        i1 = 0;
    } else {
        if (o3 != 0x10a8c || o2 != 0x10abc || o1 != 0x10aec) {
            goto L5;
        } else {
            i1 = 1;
        }
    }
    if (i1 == 0) {
        printf("Failed!\n");
    } else {
        printf("Pass\n");
    }
    return 0;
}

