int main(int argc, char *argv[]);


/** address: 0x00001be4 */
int main(int argc, char *argv[])
{
    union { long long x2; double; __size32; } f31; 		// r63
    __size32 g3; 		// r3
    float local0; 		// m[g1 - 48]

    f31 = *((float *)&*(/* machine specific */ (int) LR + 1024));
    scanf(/* machine specific */ (int) LR + 928);
    g3 = printf(/* machine specific */ (int) LR + 932);
    if (f31 == local0) {
        g3 = puts(/* machine specific */ (int) LR + 952);
    }
    if (f31 != local0) {
        g3 = puts(/* machine specific */ (int) LR + 960);
    }
    if (f31 > local0) {
        g3 = puts(/* machine specific */ (int) LR + 972);
    }
    if (f31 == local0) {
        g3 = puts(/* machine specific */ (int) LR + 980);
    }
    if (f31 == local0) {
        g3 = puts(/* machine specific */ (int) LR + 996);
    }
    if (f31 < local0) {
        g3 = puts(/* machine specific */ (int) LR + 1016);
    }
    return g3;
}

