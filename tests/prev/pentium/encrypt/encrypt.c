void rux_encrypt(unsigned char *param1);

// address: 0x8048460
int main(int argc, char *argv[], char *envp[]) {
    int eax; 		// r24
    unsigned char local0; 		// m[esp - 8]

    for(;;) {
        proc1();
        if (eax != 4) {
            break;
        }
        rux_encrypt(&local0);
        proc2();
    }
    if (eax != 0) {
        proc3();
        rux_encrypt(&local0);
        proc2();
    }
    return 0;
}

// address: 0x8048504
void rux_encrypt(unsigned char *param1) {
    unsigned char bl; 		// r11
    unsigned char bl_1; 		// r11{32}
    unsigned char cl; 		// r9
    int ecx; 		// r25
    union { __size32 * x1; int x2; } edi; 		// r31
    __size32 *esi; 		// r30
    int esp; 		// r28

    edi = (esp - 40);
    esi = 0x8048614;
    ecx = 8;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    bl_1 = *param1;
    cl = *param1;
    cl = (cl & 0xf ^ *0x8049644);
    bl = *((cl) + esp - 40);
    *(unsigned char*)param1 = bl;
    bl = (bl_1 / 16 ^ *0x8049648);
    bl = *((bl) + esp - 24);
    bl = (bl * 16 ^ *param1);
    *(unsigned char*)param1 = bl;
    proc4();
    return;
}

