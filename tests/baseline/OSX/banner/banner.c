// address: 2778
int main(int argc, char *argv[], char *envp[]) {
    union { unsigned int x283; char x284; char x282; char x280; char x278; char x276; char x274; char x272; char x270; char x268; char x266; char x264; char x262; char x260; char x258; char x256; char x254; char x252; char x250; char x248; char x246; char x244; char x242; char x240; char x238; char x236; char x234; char x232; char x230; char x228; char x226; char x224; char x222; char x220; char x218; char x216; char x214; char x212; char x210; char x206; char x204; char x200; char x198; char x196; char x194; char x190; char x188; char x186; char x184; char x180; char x178; char x176; char x174; char x170; char x168; char x166; char x164; char x160; char x158; char x156; char x154; char x150; char x148; char x146; char x144; char x140; char x138; char x136; char x134; char x130; char x128; char x126; char x124; char x120; char x118; char x116; char x114; char x110; char x108; char x106; char x104; char x100; char x98; char x96; char x94; char x90; char x88; char x86; char x84; char x80; char x78; char x76; char x74; char x70; char x68; char x66; char x64; char x60; char x58; char x56; char x54; char x50; char x48; char x46; char x44; char x40; char x38; char x36; char x34; char x30; char x28; char x26; char x24; char x18; char x16; char x14; char x12; char x8; char x6; char x4; char x2; } g0; 		// r0
    void *g0_1; 		// r0
    void *g1; 		// r1
    __size32 g10; 		// r10
    __size32 g11; 		// r11
    __size32 g3; 		// r3
    int local0; 		// m[g1 - 104]
    int local1; 		// m[g1 - 112]
    int local2; 		// m[g1 - 116]
    char local3; 		// m[g1 - 96]
    __size32 local4; 		// m[g1 - 128]
    char[] **local5; 		// m[g1 - 124]
    int local6; 		// m[g1 - 108]
    int local7; 		// m[g1 - 120]
    __size32 local8; 		// m[g1 - 128]{362}

    malloc(12);
    *(__size32*)(g3 + 4) = /* machine specific */ (int) LR + 0x1864;
    local4 = 2;
    local5 = g3 + 4;
    local8 = local4;
    local4 = local8 - 1;
    while (local8 - 1 != 0) {
        g3 = *local5;
        strlen(g3);
        local6 = g3;
        if (g3 > 10) {
            local6 = 10;
        }
        local7 = 0;
L16:
        if (local7 <= 6) {
            local2 = 0;
            while (local2 < local6) {
                g0 = *(unsigned char*)(g9 + local2);
                local0 = (int) g0 - 32;
                if ((int) g0 - 32 < 0) {
                    local0 = 0;
                }
                local1 = 0;
L2:
                if (local1 <= 6) {
                    g10 = (ROTL(local2) & 0xfffffff9) + g1 + local1 - 96;
                    g11 = (ROTL((rs * 7 + local7)) & 0xfffffffd) + /* machine specific */ (int) LR + 0x1890;
                    g0_1 = *g11;
                    g0 = *(unsigned char*)(rs * 7 + local1 + g0_1);
                    *(__size8*)g10 = (char) g0;
                    local1++;
                    goto L2;
                }
                *(__size8*)((ROTL(local2) & 0xfffffff9) + g1 - 89) = 32;
                local2++;
            }
            local2 = (ROTL(local6) & 0xfffffff9) - 1;
            while (local2 >= 0) {
                g0 = *(unsigned char*)(g1 + local2 - 96);
                if ((int) g0 != 32) {
                    break;
                }
                *(__size8*)(g1 + local2 - 96) = 0;
                local2 = local2 - 1;
            }
            puts(&local3);
            local7++;
            goto L16;
        }
        puts(/* machine specific */ (int) LR + 1108);
        local5++;
        local8 = local4;
        local4 = local8 - 1;
    }
    return 0;
}

