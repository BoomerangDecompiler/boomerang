int main(int argc, char *argv[]);
__size32 elf_hash(union { __size32; char *; } param1);

/** address: 0x00010678 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    o0 = elf_hash(0x107a8);
    printf("elf_hash(\"main\") is %x\n", o0);
    return 0;
}

/** address: 0x000106b4 */
__size32 elf_hash(union { __size32; char *; } param1)
{
    int o3; 		// r11
    int o4; 		// r12

    o3 = (int) *param1;
    o4 = 0;
    if (o3 != 0) {
        o4 = (o3 ^ (unsigned int)(o3 & 0xf0000000) >> 24) &  ~(o3 & 0xf0000000);
        o3 = (int) *(param1 + 1);
    }
    return o4;
}

