int main(int argc, char *argv[]);

/** address: 0x00010694 */
int main(int argc, char *argv[])
{
    union { __size32; float; } f8; 		// r40
    union { __size32; float; } local0; 		// m[o6 - 24]

    f8 = *0x10940;
    scanf(0x108c8);
    printf(0x108d0);
    if (f8 == local0) {
        printf(0x108e8);
    }
    else {
    }
    if (f8 != local0) {
        printf(0x108f0);
    }
    else {
    }
    if (f8 > local0) {
        printf(0x10900);
    }
    else {
    }
    if (f8 <= local0) {
        printf(0x10910);
    }
    else {
    }
    if (f8 >= local0) {
        printf(0x10920);
    }
    else {
    }
    if (f8 < local0) {
        printf(0x10938);
    }
    else {
    }
    return 0;
}

