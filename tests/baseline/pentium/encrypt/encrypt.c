void rux_encrypt(__size32 *param1);

// address: 0x8048460
int main(int argc, char *argv[], char *envp[]) {
    int eax; 		// r24
    int esp; 		// r28
    __size32 local0; 		// m[esp - 8]

    for(;;) {
        read(0, &local0, 4);
        if (eax != 4) {
            break;
        }
        rux_encrypt(&local0);
        write(1, &local0, 4);
    }
    if (eax != 0) {
        memset(esp + eax - 8, 0, 4 - eax);
        rux_encrypt(&local0);
        write(1, &local0, 4);
    }
    return 0;
}

// address: 0x8048504
void rux_encrypt(__size32 *param1) {
    unsigned char bl; 		// r11
    unsigned char bl_1; 		// r11{32}
    unsigned char cl; 		// r9
    unsigned int eax; 		// r24
    unsigned int ebx; 		// r27
    unsigned int ecx; 		// r25
    union { __size32 * x1; int x2; } edi; 		// r31
    __size32 *esi; 		// r30
    void *esp; 		// r28
    unsigned int local0; 		// m[esp - 6]

    edi = (esp - 40);
    esi = 0x8048614;
    ecx = 8;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    local0 = 0;
    while (local0 <= 3) {
        bl_1 = *((local0) + param1);
        ecx = (local0);
        cl = *(ecx + param1);
        ebx = (local0);
        cl = (cl & 0xf ^ *(ebx + 0x8049644));
        bl = *((cl) + esp - 40);
        *(unsigned char*)((local0) + param1) = bl;
        eax = (local0);
        ecx = (local0);
        bl = (bl_1 / 16 ^ *((local0) + 0x8049648));
        bl = *((bl) + esp - 24);
        bl = (bl * 16 ^ *(ecx + param1));
        *(unsigned char*)(eax + param1) = bl;
        local0++;
    }
    return;
}

