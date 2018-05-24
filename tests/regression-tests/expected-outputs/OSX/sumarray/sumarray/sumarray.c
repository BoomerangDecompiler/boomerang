int main(int argc, char *argv[]);

/** address: 0x00001d0c */
int main(int argc, char *argv[])
{
    int g0; 		// r0
    int local0; 		// m[g1 - 32]
    int local1; 		// m[g1 - 28]

    local0 = 0;
    local1 = 0;
    while (local1 <= 9) {
        g0 = *((ROTL(local1) & ~0x3) + /* machine specific */ (int) LR + 764);
        local0 += g0;
        local1++;
    }
    printf(/* machine specific */ (int) LR + 720);
    return 0;
}

