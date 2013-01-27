// address: 0x10000440
int main(int argc, char *argv[], char *envp[]) {
    double f0; 		// r32
    double f13; 		// r45
    void *g1; 		// r1
    double g4_1; 		// r4
    double g5; 		// r5
    float local0; 		// m[g1 - 20]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", g4_1, g5);
    f0 = local0;
    if (0x40a00000 == f0) {
        printf("Equal\n");
    }
    f13 = local0;
    if (0x40a00000 != f13) {
        printf("Not Equal\n");
    }
    f13 = local0;
    if (5. > f13) {
        printf("Greater\n");
    }
    f0 = local0;
    if (0x40a00000 == f0) {
        printf("Less or Equal\n");
    }
    f0 = local0;
    if (0x40a00000 == f0) {
        printf("Greater or Equal\n");
    }
    f13 = local0;
    if (5. < f13) {
        printf("Less\n");
    }
    return g1 - 20;
}

