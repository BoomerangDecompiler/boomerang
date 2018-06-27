int main(int argc, char *argv[]);

/** address: 0x00010bac */
int main(int argc, char *argv[])
{
    __size32 i0; 		// r24
    int o0; 		// r8
    int o2; 		// r10
    int o3; 		// r11
    int o4; 		// r12
    int o5; 		// r13

    o2 = 0x10d0c;
    o3 = 0x10d1c;
    o4 = 0x10d2c;
    o5 = 0x10d3c;
    i0 = 0;
    if ( ~(o2 != 0x10d0c || o3 != 0x10d1c || o4 != 0x10d2c)) {
        o0 = 0x10d3c;
        i0 = 1 - ((o5 ^ o0) != 0);
    }
    if (i0 == 0) {
        o0 = 0x118a0;
    }
    else {
        o0 = 0x11898;
    }
    printf(o0);
    return 1 - (i0 != 0);
}

