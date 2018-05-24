int main(int argc, char *argv[]);

/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    union { double; __size32; } f13; 		// r45
    __size32 g1; 		// r1
    float local0; 		// m[g1 - 20]

    scanf(0x1000091c);
    printf(0x10000920);
    if (0x40a00000 == local0) {
        printf(0x10000934);
    }
    else {
    }
    if (0x40a00000 != local0) {
        printf(0x1000093c);
    }
    else {
    }
    if (0x40a00000 > local0) {
        printf(0x10000948);
    }
    else {
    }
    if (0x40a00000 == local0) {
        printf(0x10000954);
    }
    else {
    }
    if (0x40a00000 == local0) {
        printf(0x10000964);
    }
    else {
    }
    f13 = local0;
    if (0x40a00000 < local0) {
        printf(0x10000978);
    }
    else {
    }
    return g1 - 20;
}

