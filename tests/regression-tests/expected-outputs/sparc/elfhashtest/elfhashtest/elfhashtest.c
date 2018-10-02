int main(int argc, char *argv[]);
__size32 elf_hash(union { __size32; char *; } param1, int param2);

/** address: 0x00010678 */
int main(int argc, char *argv[])
{
    int o0; 		// r8
    int o2; 		// r10

    o0 = elf_hash(0x107a8, o2);
    printf("elf_hash(\"main\") is %x\n", o0);
    return 0;
}

/** address: 0x000106b4 */
__size32 elf_hash(union { __size32; char *; } param1, int param2)
{
    __size32 g1; 		// r1
    __size32 g1_1; 		// r1{0}
    int local0; 		// param2{0}
    int local1; 		// o2{0}
    int local2; 		// o3{0}
    int o2; 		// r10
    int o3; 		// r11
    int o3_1; 		// r11{0}
    int o3_2; 		// r11{0}
    unsigned int o4; 		// r12
    unsigned int o4_1; 		// r12{0}
    int o4_2; 		// r12{0}

    o3 = (int) *param1;
    g1 = param1;
    o4 = 0;
    local0 = param2;
    if (o3 != 0) {
        do {
            g1_1 = g1;
            param2 = local0;
            o3_1 = o3;
            o4_1 = o4;
            o3_2 = o4_1 * 16 + o3_1;
            g1 = g1_1 + 1;
            o4_2 = o4_1 * 16 + o3_1 & 0xf0000000;
            local1 = param2;
            local2 = o3_2;
            if (o4_2 != 0) {
                o2 = (unsigned int)o4_2 >> 24;
                o3 = o4_1 * 16 + o3_1 ^ (unsigned int)o4_2 >> 24;
                local1 = o2;
                local2 = o3;
            }
            o2 = local1;
            o3 = local2;
            o4 = o3 &  ~o4_2;
            o3 = (int) *(g1_1 + 1);
            local0 = o2;
        } while (o3 != 0);
    }
    return o4;
}

