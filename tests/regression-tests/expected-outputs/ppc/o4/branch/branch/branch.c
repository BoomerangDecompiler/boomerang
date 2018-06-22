int main(int argc, char *argv[]);

/** address: 0x1000043c */
int main(int argc, char *argv[])
{
    __size32 g3; 		// r3
    int local0; 		// m[g1 - 24]
    unsigned int local1; 		// m[g1 - 20]

    scanf(0x10000990);
    scanf(0x10000990);
    if (local0 == 5) {
        g3 = puts(0x100009a8);
        if (local0 != 5) {
            g3 = puts(0x10000994);
        }
        if (5 > local0) {
bb0x100005b4:
            g3 = puts(0x10000a18);
        }
        if (5 <= local0) {
bb0x10000498:
            g3 = puts(0x100009a0);
        }
    }
    else {
        puts(0x10000994);
        if (5 > local0) {
            goto bb0x100005b4;
        }
        else {
            goto bb0x10000498;
        }
    }
    if (5 >= local0) {
        g3 = puts(0x100009b0);
    }
    if (5 < local0) {
        g3 = puts(0x100009c4);
    }
    if ((unsigned int)5 > local1) {
        g3 = puts(0x10000a04);
    }
    if ((unsigned int)5 <= local1) {
        g3 = puts(0x100009ec);
    }
    if ((unsigned int)5 >= local1) {
        g3 = puts(0x100009e0);
    }
    if ((unsigned int)5 < local1) {
        g3 = puts(0x100009d4);
    }
    if (5 >= local0) {
        g3 = puts(0x100009cc);
    }
    if (5 < local0) {
        g3 = puts(0x10000a20);
    }
    return g3;
}

