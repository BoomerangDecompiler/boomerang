int main(int argc, char *argv[]);
void rux_encrypt(__size32 param1);

/** address: 0x08048460 */
int main(int argc, char *argv[])
{
    size_t eax; 		// r24
    int esp; 		// r28
    int local0; 		// m[esp - 8]

    for(;;) {
        eax = read(0, &local0, 4);
        if (eax == 4) {
            rux_encrypt(esp - 8);
            write(1, &local0, 4);
        }
    }
    if (eax != 0) {
        memset(esp + eax - 8, 0, 4 - eax);
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
    __size32 edi; 		// r31
    __size32 esi; 		// r30
    __size32 esp; 		// r28
    int local0; 		// m[esp - 6]

    edi = (esp - 40);
    esi = 0x8048614;
    ecx = 8;
    *(__size32*)edi = *esi;
    esi +=  (DF == 0) ? 4 : -4;
    edi +=  (DF == 0) ? 4 : -4;
    ecx = ecx - 1;
    local0 = 0;
    while (local0 <= (unsigned int)3) {
        bl_1 = *((local0) + param1);
        cl = *((local0) + param1);
        cl = (cl & 0xf ^ *((local0) + 0x8049644));
        bl_4 = *((cl) + esp - 40);
        *(unsigned char*)((local0) + param1) = bl_4;
        eax = (local0);
        ecx = (local0);
        bl = (bl_1 / 16 ^ *((local0) + 0x8049648));
        bl = *((bl) + esp - 24);
        bl = (bl * 16 ^ *((local0) + param1));
        *(unsigned char*)((local0) + param1) = bl;
        local0++;
    }
    return;
}

