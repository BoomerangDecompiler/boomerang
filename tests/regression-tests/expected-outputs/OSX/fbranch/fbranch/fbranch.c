int main(int argc, char *argv[]);

/** address: 0x00001bb8 */
int main(int argc, char *argv[])
{
    union { double; __size32; } f13; 		// r45
    __size32 g1; 		// r1
    float local0; 		// m[g1 - 44]

    scanf(/* machine specific */ (int) LR + 972);
    printf(/* machine specific */ (int) LR + 976);
    if (0x40a00000 == local0) {
        printf(/* machine specific */ (int) LR + 996);
    }
    else {
    }
    if (0x40a00000 != local0) {
        printf(/* machine specific */ (int) LR + 1004);
    }
    else {
    }
    if (5. > local0) {
        printf(/* machine specific */ (int) LR + 1016);
    }
    else {
    }
    if (0x40a00000 == local0) {
        printf(/* machine specific */ (int) LR + 1028);
    }
    else {
    }
    if (0x40a00000 == local0) {
        printf(/* machine specific */ (int) LR + 1044);
    }
    else {
    }
    f13 = local0;
    if (5. < local0) {
        printf(/* machine specific */ (int) LR + 1064);
    }
    else {
    }
    return g1 - 44;
}

