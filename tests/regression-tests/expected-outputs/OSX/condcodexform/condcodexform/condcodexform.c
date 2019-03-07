int main(int argc, char *argv[]);

__size32 global_0x00002020;// 4 bytes
__size32 global_0x00002024;// 4 bytes
__size32 global_0x00002028;// 4 bytes
__size32 global_0x0000202c;// 4 bytes
__size32 global_0x00002030;// 4 bytes
__size32 global_0x00002034;// 4 bytes
__size32 global_0x00002038;// 4 bytes
__size32 global_0x0000203c;// 4 bytes

/** address: 0x00001b70 */
int main(int argc, char *argv[])
{
    int CR0; 		// r100
    int CR1; 		// r101
    int CR2; 		// r102
    int CR3; 		// r103
    int CR4; 		// r104
    int CR5; 		// r105
    int CR6; 		// r106
    __size32 CR7; 		// r107
    __size32 local0; 		// m[g1 - 40]
    __size32 local1; 		// m[g1 - 36]
    __size32 local2; 		// m[g1 - 32]
    __size32 local3; 		// m[g1 - 28]
    __size32 local4; 		// m[g1 - 24]
    __size32 local5; 		// m[g1 - 48]

    local0 = 1;
    if (argc <= 1) {
        local0 = 0;
    }
    if (local0 == 0) {
        local1 = global_0x00002038;
    }
    else {
        local1 = global_0x0000203c;
    }
    if (local0 == 0) {
        local2 = global_0x00002030;
    }
    else {
        local2 = global_0x00002034;
    }
    if (local0 == 0) {
        local3 = global_0x00002028;
    }
    else {
        local3 = global_0x0000202c;
    }
    if (local0 == 0) {
        local4 = global_0x00002020;
    }
    else {
        local4 = global_0x00002024;
    }
    if (argc <= 1) {
        local5 = 0;
        if (local1 == global_0x00002038 && local2 == global_0x00002030 && local3 == global_0x00002028 && local4 == global_0x00002020) {
            local5 = 1;
        }
    }
    else {
        local5 = 0;
        if (local1 == global_0x0000203c && local2 == global_0x00002034 && local3 == global_0x0000202c && local4 == global_0x00002024) {
            local5 = 1;
        }
    }
    if (local5 == 0) {
        printf("Failed!\n");
    }
    else {
        printf("Pass\n");
    }
    return ROTL(((CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7), 31) & 0x1;
}

