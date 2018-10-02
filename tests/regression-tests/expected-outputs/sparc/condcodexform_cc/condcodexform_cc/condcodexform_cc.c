int main(int argc, char *argv[]);

/** address: 0x00010b0c */
int main(int argc, char *argv[])
{
    __size32 i0; 		// r24
    __size32 i1; 		// r25
    __size32 local0; 		// o0_2{0}
    int o0; 		// r8
    __size32 o0_1; 		// r8{0}
    __size32 o0_2; 		// r8{0}
    __size32 o0_3; 		// r8{0}
    int o1; 		// r9
    int o2; 		// r10
    int o3; 		// r11

    o0 = 1;
    if (argc <= 1) {
        o0 = 0;
    }
    o0_1 = o0;
    if (o0_1 == 0) {
        o0 = 0x10a74;
        local0 = o0;
    }
    else {
        o0_3 = 0x10a5c;
        local0 = o0_3;
    }
    o0_2 = local0;
    if (o0_1 == 0) {
        o3 = 0x10aa4;
    }
    else {
        o3 = 0x10a8c;
    }
    if (o0_1 == 0) {
        o2 = 0x10ad4;
    }
    else {
        o2 = 0x10abc;
    }
    if (o0_1 == 0) {
        o1 = 0x10b04;
    }
    else {
        o1 = 0x10aec;
    }
    if (o0_1 == 0) {
        if (o0_2 != 0x10a74 || o3 != 0x10aa4 || o2 != 0x10ad4 || o1 != 0x10b04) {
            i1 = 0;
        }
        else {
            i1 = 1;
        }
    }
    else {
        if (o0_2 != 0x10a5c) {
bb0x10be0:
            i1 = 0;
        }
        else {
            if (o3 != 0x10a8c || o2 != 0x10abc || o1 != 0x10aec) {
                goto bb0x10be0;
            }
            else {
                i1 = 1;
            }
        }
    }
    if (i1 == 0) {
        printf("Failed!\n");
    }
    else {
        printf("Pass\n");
    }
    i0 = 0;
    if (i1 == 0) {
        i0 = 1;
    }
    return i0;
}

