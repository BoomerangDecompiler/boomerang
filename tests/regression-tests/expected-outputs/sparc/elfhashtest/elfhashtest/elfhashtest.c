int main(int argc, char *argv[]);
__size32 elf_hash(char *param1, int param2);


/** address: 0x00010678 */
int main(int argc, char *argv[])
{
    unsigned int o0_1; 		// r8
    int o2; 		// r10

    o0_1 = elf_hash("main", o2);
    printf("elf_hash(\"main\") is %x\n", o0_1);
    return 0;
}

/** address: 0x000106b4 */
__size32 elf_hash(char *param1, int param2)
{
    char *g1; 		// r1
    char *g1_1; 		// r1{7}
    int local0; 		// param2{8}
    int local1; 		// o2{17}
    int local2; 		// o3{18}
    int o2; 		// r10
    int o3; 		// r11
    int o3_1; 		// r11{9}
    int o3_2; 		// r11{11}
    int o4; 		// r12
    int o4_1; 		// r12{10}
    int o4_2; 		// r12{13}

    local0 = param2;
    o3 = (int) *param1;
    g1 = param1;
    o4 = 0;
    if (o3 != 0) {
        do {
            g1_1 = g1;
            param2 = local0;
            local1 = param2;
            o3_1 = o3;
            o4_1 = o4;
            o3_2 = (o4_1 << 4) + o3_1;
            local2 = o3_2;
            g1 = g1_1 + 1;
            o4_2 = (o4_1 << 4) + o3_1 & 0xf0000000;
            if (((o4_1 << 4) + o3_1 & 0xf0000000) != 0) {
                o2 = (unsigned int)o4_2 >> 24;
                local1 = o2;
                o3 = (o4_1 << 4) + o3_1 ^ (unsigned int)o4_2 >> 24;
                local2 = o3;
            }
            o2 = local1;
            local0 = o2;
            o3 = local2;
            o4 = o3 & ~o4_2;
            o3 = (int) *(g1_1 + 1);
        } while (o3 != 0);
    }
    return o4;
}

