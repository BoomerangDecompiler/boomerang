// address: 0x1bb8
int main(int argc, char *argv[], char *envp[]) {
    double f0; 		// r32
    double f13; 		// r45
    void *g1; 		// r1
    float local0; 		// m[g1 - 44]

    scanf(/* machine specific */ (int) LR + 972);
    printf(/* machine specific */ (int) LR + 976);
    f0 = local0;
    if (0x40a00000 == f0) {
        printf(/* machine specific */ (int) LR + 996);
    }
    f13 = local0;
    if (0x40a00000 != f13) {
        printf(/* machine specific */ (int) LR + 1004);
    }
    f13 = local0;
    if (5. > f13) {
        printf(/* machine specific */ (int) LR + 1016);
    }
    f0 = local0;
    if (0x40a00000 == f0) {
        printf(/* machine specific */ (int) LR + 1028);
    }
    f0 = local0;
    if (0x40a00000 == f0) {
        printf(/* machine specific */ (int) LR + 1044);
    }
    f13 = local0;
    if (5. < f13) {
        printf(/* machine specific */ (int) LR + 1064);
    }
    return g1 - 44;
}

