int main(int argc, char *argv[]);

/** address: 0x10000468 */
int main(int argc, char *argv[])
{
    union { double; __size64; } f0; 		// r32
    union { double; __size32; } f1; 		// r33
    __size32 g3; 		// r3
    float local0; 		// m[g1 - 24]

    scanf(0x1000091c);
    printf(0x10000920);
    f1 = local0;
    if (5. == local0) {
        g3 = puts(0x10000958);
        if (5. != local0) {
bb0x100004c8:
            g3 = puts(0x10000934);
        }
    }
    else {
        goto bb0x100004c8;
    }
    if (5. > local0) {
        g3 = puts(0x10000940);
    }
    f0 = local0;
    if (5. == f0) {
        g3 = puts(0x10000950);
        if (5. == local0) {
bb0x10000568:
            g3 = puts(0x10000960);
        }
    }
    else {
        if (5. == f0) {
            goto bb0x10000568;
        }
    }
    f0 = local0;
    if (5. < local0) {
        g3 = puts(0x10000948);
    }
    return g3;
}

