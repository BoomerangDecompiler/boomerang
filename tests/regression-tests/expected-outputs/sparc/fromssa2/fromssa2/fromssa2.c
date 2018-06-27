int main(int argc, char *argv[]);

/** address: 0x00010684 */
int main(int argc, char *argv[])
{
    __size32 local0; 		// o0_1{0}
    int o0; 		// r8
    __size32 o0_1; 		// r8{0}
    __size32 o0_4; 		// r8{0}

    o0 = 0;
    local0 = o0;
    do {
        o0_1 = local0;
        printf("%d ", o0_1 + 1);
        o0_4 = o0_1 + 1;
        local0 = o0_4;
    } while (o0_1 + 1 <= 9);
    printf("a is %d, x is %d\n", o0_1 + 1, o0_1 + 1);
    return 0;
}

