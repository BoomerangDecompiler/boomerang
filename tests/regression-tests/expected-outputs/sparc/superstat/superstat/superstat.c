int main(int argc, union { __size32; char *[] *; } argv);

/** address: 0x000106a8 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    struct stat local0; 		// m[o6 - 152]
    int o0; 		// r8

    o0 = *(argv + 4);
    stat(o0, &local0);
    printf(0x10840);
    printf(0x10850);
    printf(0x10860);
    printf(0x10870);
    printf(0x10880);
    printf(0x10890);
    printf(0x108a0);
    printf(0x108b0);
    printf(0x108c0);
    printf(0x108d0);
    printf(0x108e0);
    printf(0x108f0);
    printf(0x10900);
    printf(0x10910);
    return 0;
}

