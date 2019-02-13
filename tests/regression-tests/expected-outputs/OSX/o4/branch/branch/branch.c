int main(int argc, char *argv[]);


/** address: 0x00001b78 */
int main(int argc, char *argv[])
{
    __size32 g3; 		// r3
    int local0; 		// m[g1 - 32]
    unsigned int local1; 		// m[g1 - 28]

    scanf(/* machine specific */ (int) LR + 988);
    scanf(/* machine specific */ (int) LR + 988);
    if (local0 == 5) {
        puts(/* machine specific */ (int) LR + 992);
    }
    if (local0 != 5) {
        puts(/* machine specific */ (int) LR + 1000);
    }
    if (5 > local0) {
        puts(/* machine specific */ (int) LR + 1012);
    }
    if (5 <= local0) {
        puts(/* machine specific */ (int) LR + 1020);
    }
    if (5 >= local0) {
        puts(/* machine specific */ (int) LR + 1036);
    }
    if (5 < local0) {
        puts(/* machine specific */ (int) LR + 1056);
    }
    if (5 > local1) {
        puts(/* machine specific */ (int) LR + 1064);
    }
    if (5 <= local1) {
        puts(/* machine specific */ (int) LR + 1084);
    }
    if (5 >= local1) {
        puts(/* machine specific */ (int) LR + 1108);
    }
    if (5 < local1) {
        puts(/* machine specific */ (int) LR + 1120);
    }
    if (5 >= local0) {
        puts(/* machine specific */ (int) LR + 1132);
    }
    g3 = 5 - local0;
    if (5 < local0) {
        g3 = puts(/* machine specific */ (int) LR + 1140);
    }
    return g3;
}

