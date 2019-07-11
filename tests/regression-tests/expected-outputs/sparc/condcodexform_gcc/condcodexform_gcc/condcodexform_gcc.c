int main(int argc, char *argv[]);


/** address: 0x00010bac */
int main(int argc, char *argv[])
{
    __size32 i0; 		// r24
    int local0; 		// argc{39}
    int o0; 		// r8
    char *o0_1; 		// r8
    int o1; 		// r9
    int o2; 		// r10
    int o3; 		// r11
    int o4; 		// r12
    int o5; 		// r13

    local0 = argc;
    local0 = argc;
    if (argc > 1) {
        o1 = 1;
    }
    else {
        o1 = 0;
    }
    if (o1 == 0) {
        o2 = 0x10d0c;
    }
    else {
        o2 = 0x10d04;
    }
    if (o1 == 0) {
        o3 = 0x10d1c;
    }
    else {
        o3 = 0x10d14;
    }
    if (o1 == 0) {
        o4 = 0x10d2c;
    }
    else {
        o4 = 0x10d24;
    }
    if (o1 == 0) {
        o5 = 0x10d3c;
    }
    else {
        o5 = 0x10d34;
    }
    if (o1 == 0) {
        i0 = 0;
        if (o2 == 0x10d0c && o3 == 0x10d1c && o4 == 0x10d2c) {
            o0 = 0x10d3c;
bb0x10cc4:
            i0 = 1 - ((o5 ^ o0) != 0);
            local0 = o0;
        }
    }
    else {
        i0 = 0;
        if (o2 == 0x10d04 && o3 == 0x10d14 && o4 == 0x10d24) {
            o0 = 0x10d34;
            goto bb0x10cc4;
        }
    }
    argc = local0;
    if (i0 == 0) {
        o0_1 = "Failed!\n";
    }
    else {
        o0_1 = "Pass\n";
    }
    printf(o0_1);
    return 1 - (i0 != 0);
}

