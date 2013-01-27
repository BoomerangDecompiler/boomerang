// address: 0x10bac
int main(int argc, char *argv[], char *envp[]) {
    unsigned int i0_1; 		// r24{77}
    int o0; 		// r8
    char *o0_1; 		// r8
    int o2; 		// r10
    int o3; 		// r11
    int o4; 		// r12
    int o5; 		// r13

    o2 = 0x10d0c;
    o3 = 0x10d1c;
    o4 = 0x10d2c;
    o5 = 0x10d3c;
    i0_1 = 0;
    if ( !(o2 != 0x10d0c || o3 != 0x10d1c || o4 != 0x10d2c)) {
        o0 = 0x10d3c;
        i0_1 = 1 - (0 < (unsigned int)(o5 ^ o0));
    }
    if (i0_1 == 0) {
        o0_1 = "Failed!\n";
    } else {
        o0_1 = "Pass\n";
    }
    printf(o0_1);
    return 1 - (0 < i0_1);
}

