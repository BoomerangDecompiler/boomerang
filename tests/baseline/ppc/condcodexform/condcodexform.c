// address: 100004f8
int main(int argc, char *argv[], char *envp[]) {
    unsigned int CR0; 		// r64
    unsigned int CR1; 		// r65
    unsigned int CR2; 		// r66
    unsigned int CR3; 		// r67
    unsigned int CR4; 		// r68
    unsigned int CR5; 		// r69
    unsigned int CR6; 		// r70
    __size32 CR7; 		// r71
    __size32 local0; 		// m[g1 - 40]
    __size32 local1; 		// m[g1 - 36]
    __size32 local2; 		// m[g1 - 32]
    __size32 local3; 		// m[g1 - 28]
    __size32 local4; 		// m[g1 - 24]
    __size32 local6; 		// m[g1 - 48]{72}

    local0 = 1;
    if (argc <= 1) {
        local0 = 0;
    }
    if (local0 == 0) {
        local1 = 0x10000434;
    } else {
        local1 = 0x10000418;
    }
    if (local0 == 0) {
        local2 = 0x1000046c;
    } else {
        local2 = 0x10000450;
    }
    if (local0 == 0) {
        local3 = 0x100004a4;
    } else {
        local3 = 0x10000488;
    }
    if (local0 == 0) {
        local4 = 0x100004dc;
    } else {
        local4 = 0x100004c0;
    }
    if (argc <= 1) {
        local6 = 0;
        if ( !(local1 != 0x10000434 || local2 != 0x1000046c || local3 != 0x100004a4 || local4 != 0x100004dc)) {
            local6 = 1;
        }
    } else {
        local6 = 0;
        if ( !(local1 != 0x10000418 || local2 != 0x10000450 || local3 != 0x10000488 || local4 != 0x100004c0)) {
            local6 = 1;
        }
    }
    if (local6 == 0) {
        printf("Failed!\n");
    } else {
        printf("Pass\n");
    }
    return ROTL((CR0 * 0x10000000 + CR1 * 0x1000000 + CR2 * 0x100000 + CR3 * 0x10000 + CR4 * 0x1000 + CR5 * 256 + CR6 * 16 + CR7)) & 0x1;
}

