int main(int argc, char *argv[]);
void rux_encrypt(__size32 param1);

/** address: 0x08048460 */
int main(int argc, char *argv[])
{
    size_t eax_1; 		// r24{0}
    int esp; 		// r28
    int local0; 		// m[esp - 8]

    for(;;) {
        eax_1 = read(0, &local0, 4);
        if (eax_1 == 4) {
            rux_encrypt(esp - 8);
            write(1, &local0, 4);
        }
    }
    if (eax_1 != 0) {
        memset(esp + eax_1 - 8, 0, 4 - eax_1);
        rux_encrypt(esp - 8);
        write(1, &local0, 4);
    }
    return 0;
}

/** address: 0x08048504 */
void rux_encrypt(__size32 param1)
{
    unsigned char bl; 		// r11
    unsigned char bl_1; 		// r11{0}
    unsigned char bl_4; 		// r11{0}
    unsigned char cl; 		// r9
    unsigned int eax; 		// r24
    unsigned int ecx; 		// r25
    __size32 esp; 		// r28
    unsigned int local0; 		// m[esp - 6]

    local0 = 0;
    while (local0 <= 3) {
        bl_1 = *((local0) + param1);
        cl = *((local0) + param1);
        cl = (cl & 0xf ^ *((local0) + 0x8049644));
        bl_4 = *((cl) + esp - 40);
        *(unsigned char*)((local0) + param1) = bl_4;
        eax = (local0);
        ecx = (local0);
        bl = (bl_1 >> 4 ^ *((local0) + 0x8049648));
        bl = *((bl) + esp - 24);
        bl = (bl << 4 ^ *((local0) + param1));
        *(unsigned char*)((local0) + param1) = bl;
        local0++;
    }
    return;
}

